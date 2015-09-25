#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, Client* client, std::string id, std::string token, SceneInfosDto dto)
		: _id(id),
		_peer(connection),
		_token(token),
		_client(client),
		_metadata(dto.Metadata)
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = Route_ptr(new Route(this, routeDto.Name, routeDto.Handle, routeDto.Metadata));
		}
	}

	Scene::~Scene()
	{
		disconnect();

		_client->_pluginCtx.sceneDeleted(this);

		for (auto sub : subscriptions)
		{
			sub.unsubscribe();
		}

		if (_host)
		{
			delete _host;
		}
	}

	std::string Scene::getHostMetadata(std::string key)
	{
		return _metadata[key];
	}

	byte Scene::handle()
	{
		return _handle;
	}

	std::string Scene::id()
	{
		return _id;
	}

	bool Scene::connected()
	{
		return _connected;
	}

	IConnection* Scene::hostConnection()
	{
		return _peer;
	}

	std::vector<Route_ptr> Scene::localRoutes()
	{
		return mapValues(_localRoutesMap);
	}

	std::vector<Route_ptr> Scene::remoteRoutes()
	{
		return mapValues(_remoteRoutesMap);
	}

	void Scene::addRoute(std::string routeName, std::function<void(Packetisp_ptr)> handler, stringMap metadata)
	{
		if (routeName.length() == 0 || routeName[0] == '@')
		{
			throw std::invalid_argument("A route cannot be empty or start with the '@' character.");
		}

		if (_connected)
		{
			throw std::runtime_error("You cannot register handles once the scene is connected.");
		}

		if (!mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route_ptr(new Route(this, routeName, metadata));
		}

		subscriptions.push_back(onMessage(routeName).subscribe(handler));
	}

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(Route_ptr route)
	{
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([this, route](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto handler = std::function<void(Packet_ptr)>([this, subscriber](Packet_ptr p) {
				auto metadata = p->metadata();
				Packetisp_ptr packet(new Packet<IScenePeer>(host(), p->stream, metadata));
				subscriber.on_next(packet);
			});
			route->handlers.push_back(handler);
			auto it = route->handlers.end();
			it--;
			subscriber.add([route, it]() {
				route->handlers.erase(it);
			});
		});
		return observable.as_dynamic();
	}

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(std::string routeName)
	{
		if (_connected)
		{
			throw std::runtime_error("You cannot register handles once the scene is connected.");
		}

		Route_ptr route;
		if (!mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route_ptr(new Route(this, routeName));
		}

		route = _localRoutesMap[routeName];
		return onMessage(route);
	}

	void Scene::sendPacket(std::string routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (!routeName.length())
		{
			throw std::invalid_argument("routeName is empty.");
		}

		if (!_connected)
		{
			throw std::runtime_error("The scene must be connected to perform this operation.");
		}

		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(std::string("The route '") + routeName + "' doesn't exist on the scene.");
		}
		Route_ptr route = _remoteRoutesMap[routeName];

		_peer->sendToScene(_handle, route->_handle, writer, priority, reliability);
	}

	pplx::task<void> Scene::connect()
	{
		if (!_connected && !connectCalled)
		{
			connectCalled = true;
			connectTask = _client->connectToScene(this, _token, mapValues(_localRoutesMap));
			connectTask.then([this]() {
				this->_connected = true;
			});
		}
		return connectTask;
	}

	pplx::task<void> Scene::disconnect()
	{
		if (_connected && !disconnectCalled)
		{
			disconnectCalled = true;
			disconnectTask = _client->disconnect(this, _handle);
			this->_connected = false;
		}
		return disconnectTask;
	}

	void Scene::completeConnectionInitialization(ConnectionResult& cr)
	{
		_handle = cr.SceneHandle;

		for (auto pair : _localRoutesMap)
		{
			pair.second->_handle = cr.RouteMappings[pair.first];
			_handlers[pair.second->_handle] = pair.second->handlers;
		}
	}

	void Scene::handleMessage(Packet_ptr packet)
	{
		packetReceived(packet);

		uint16 routeId;
		*packet->stream >> routeId;

		packet->metadata()["routeId"] = new uint16(routeId);

		if (mapContains(_handlers, routeId))
		{
			auto observers = _handlers[routeId];
			for (auto f : observers)
			{
				f(packet);
			}
		}

		delete packet->metadata()["routeId"];
	}

	void Scene::registerComponent(std::string componentName, std::function<void*()> factory)
	{
		_registeredComponents[componentName] = factory;
	}

	void* Scene::getComponent(std::string componentName)
	{
		if (!mapContains(_registeredComponents, componentName))
		{
			throw std::runtime_error(std::string(componentName) + " component not found.");
		}
		return _registeredComponents[componentName]();
	}

	std::vector<IScenePeer*> Scene::remotePeers()
	{
		return std::vector < IScenePeer* > {host()};
	}

	IScenePeer* Scene::host()
	{
		if (!_host)
		{
			_host = new ScenePeer(_peer, _handle, _remoteRoutesMap, this);
		}
		return _host;
	}
};
