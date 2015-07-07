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
			_remoteRoutesMap[routeDto.Name] = Route(this, routeDto.Name, routeDto.Handle, routeDto.Metadata);
		}
	}

	Scene::~Scene()
	{
		disconnect();

		if (_host)
		{
			delete _host;
		}

		for (auto sub : subscriptions)
		{
			sub.unsubscribe();
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

	std::vector<Route*> Scene::localRoutes()
	{
		return mapValuesPtr(_localRoutesMap);
	}

	std::vector<Route*> Scene::remoteRoutes()
	{
		return mapValuesPtr(_remoteRoutesMap);
	}

	void Scene::addRoute(std::string routeName, std::function<void(std::shared_ptr<Packet<IScenePeer>>)> handler, stringMap metadata)
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
			_localRoutesMap[routeName] = Route(this, routeName, metadata);
		}

		subscriptions.push_back(onMessage(routeName).subscribe(handler));
	}

	rxcpp::observable<std::shared_ptr<Packet<IScenePeer>>> Scene::onMessage(Route* route)
	{
		auto observable = rxcpp::observable<>::create<std::shared_ptr<Packet<IScenePeer>>>([this, route](rxcpp::subscriber<std::shared_ptr<Packet<IScenePeer>>> subscriber) {
			auto handler = new std::function<void(std::shared_ptr<Packet<>>)>([this, subscriber](std::shared_ptr<Packet<>> p) {
				auto metadata = p->metadata();
				std::shared_ptr<Packet<IScenePeer>> packet(new Packet<IScenePeer>(host(), p->stream, metadata));
				subscriber.on_next(packet);
			});
			route->handlers.push_back(handler);

			subscriber.add([route, handler]() {
				auto it = std::find(route->handlers.begin(), route->handlers.end(), handler);
				route->handlers.erase(it);
				delete handler;
			});
		});
		return observable.as_dynamic();
	}

	rxcpp::observable<std::shared_ptr<Packet<IScenePeer>>> Scene::onMessage(std::string routeName)
	{
		if (_connected)
		{
			throw std::runtime_error("You cannot register handles once the scene is connected.");
		}

		if (!mapContains(_localRoutesMap, routeName))
		{
			_localRoutesMap[routeName] = Route(this, routeName);
		}

		Route* route = &_localRoutesMap[routeName];
		return onMessage(route);
	}

	void Scene::sendPacket(std::string routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (routeName.length() == 0)
		{
			throw std::invalid_argument("routeName is empty.");
		}
		if (!_connected)
		{
			throw std::runtime_error("The scene must be connected to perform this operation.");
		}

		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(stringFormat("The route '", routeName, "' doesn't exist on the scene.").c_str());
		}
		Route& route = _remoteRoutesMap[routeName];

		_peer->sendToScene(_handle, route._handle, writer, priority, reliability);
	}

	pplx::task<void> Scene::connect()
	{
		if (!_connected && !connectCalled)
		{
			connectCalled = true;
			connectTask = _client->connectToScene(this, _token, mapValuesPtr(_localRoutesMap));
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
			pair.second._handle = cr.RouteMappings[pair.first];
			_handlers[pair.second._handle] = pair.second.handlers;
		}
	}

	void Scene::handleMessage(std::shared_ptr<Packet<>> packet)
	{
		packetReceived(packet);

		uint16 routeId;
		*packet->stream >> routeId;

		packet->setMetadata("routeId", new uint16(routeId));

		if (mapContains(_handlers, routeId))
		{
			auto observers = _handlers[routeId];
			for (size_t i = 0; i < observers.size(); i++)
			{
				(*(observers[i]))(packet);
			}
		}

		delete packet->getMetadata<uint16>("routeId");
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
