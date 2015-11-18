#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, Client* client, std::string id, std::string token, SceneInfosDto dto, DependencyResolver* parentDependencyResolver, Action<void> onDelete)
		: _id(id),
		_peer(connection),
		_token(token),
		_client(client),
		_metadata(dto.Metadata),
		_onDelete(onDelete),
		_dependencyResolver(new DependencyResolver(parentDependencyResolver))
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = Route_ptr(new Route(this, routeDto.Name, routeDto.Handle, routeDto.Metadata));
		}
	}

	Scene::~Scene()
	{
		_onDelete();

		disconnect();

		for (auto sub : subscriptions)
		{
			sub.unsubscribe();
		}

		if (_host)
		{
			delete _host;
		}
	}
	
	void Scene::destroy()
	{
		delete this;
	}

	const char* Scene::getHostMetadata(const char* key)
	{
		return _metadata[key].c_str();
	}

	byte Scene::handle()
	{
		return _handle;
	}

	const char* Scene::id()
	{
		return _id.c_str();
	}

	bool Scene::connected()
	{
		return _connected;
	}

	IConnection* Scene::hostConnection()
	{
		return _peer;
	}

	DataStructures::List<Route_ptr> Scene::localRoutes()
	{
		return mapToRakListValues(_localRoutesMap);
	}

	DataStructures::List<Route_ptr> Scene::remoteRoutes()
	{
		return mapToRakListValues(_remoteRoutesMap);
	}

	void Scene::addRoute(const char* routeName2, std::function<void(Packetisp_ptr)> handler, std::map<const char*, const char*>* metadata2)
	{
		std::string routeName = routeName2;

		stringMap metadata;
		if (metadata2)
		{
			for (auto it : *metadata2)
			{
				metadata[std::string(it.first)] = std::string(it.second);
			}
		}

		if (routeName.size() == 0 || routeName[0] == '@')
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

		subscriptions.push_back(onMessage(routeName2).subscribe(handler));
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

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(const char* routeName2)
	{
		std::string routeName = routeName2;
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

	void Scene::sendPacket(const char* routeName2, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		if (!std::strlen(routeName2))
		{
			throw std::invalid_argument("routeName is empty.");
		}

		if (!_connected)
		{
			throw std::runtime_error("The scene must be connected to perform this operation.");
		}

		std::string routeName = routeName2;
		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(std::string("The route '") + routeName + "' doesn't exist on the scene.");
		}
		Route_ptr route = _remoteRoutesMap[routeName];

		_peer->sendToScene(_handle, route->handle(), writer, priority, reliability);
	}

	pplx::task<void> Scene::connect()
	{
		if (!_connected && !_connecting && _client)
		{
			_connecting = true;
			_connectTask = _client->connectToScene(this, _token, mapValues(_localRoutesMap));
			_connectTask.then([this]() {
				_connected = true;
			});
		}
		return _connectTask;
	}

	pplx::task<void> Scene::disconnect()
	{
		if (_connected && _client)
		{
			_connected = false;
			_disconnectTask = _client->disconnect(this, _handle);
		}
		return _disconnectTask;
	}

	void Scene::completeConnectionInitialization(ConnectionResult& cr)
	{
		_handle = cr.SceneHandle;

		for (auto pair : _localRoutesMap)
		{
			pair.second->setHandle(cr.RouteMappings[pair.first]);
			_handlers[pair.second->handle()] = pair.second->handlers;
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

	DependencyResolver* Scene::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	void Scene::registerComponent(const char* componentName, std::function<void*()> factory)
	{
		_registeredComponents[componentName] = factory;
	}

	void* Scene::getComponent(const char* componentName2)
	{
		std::string componentName = componentName2;
		if (!mapContains(_registeredComponents, componentName))
		{
			throw std::runtime_error("Component not found (" + std::string(componentName) + ")");
		}
		return _registeredComponents[componentName]();
	}

	DataStructures::List<IScenePeer*> Scene::remotePeers()
	{
		DataStructures::List<IScenePeer*> container;
		container.Insert(host(), __FILE__, __LINE__);
		return container;
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
