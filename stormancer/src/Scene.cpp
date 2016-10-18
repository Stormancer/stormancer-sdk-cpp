#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, Client* client, std::string id, std::string token, SceneInfosDto dto, DependencyResolver* parentDependencyResolver, std::function<void()> onDelete)
		: _id(id)
		, _peer(connection)
		, _token(token)
		, _client(client)
		, _metadata(dto.Metadata)
		, _onDelete(onDelete)
		, _dependencyResolver(new DependencyResolver(parentDependencyResolver))
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = Route_ptr(new Route(this, routeDto.Name, routeDto.Handle, routeDto.Metadata));
		}

		_connectionStateObservable.get_observable().subscribe([this](ConnectionState state) {
			this->_connectionState = state;
		});
	}

	Scene::~Scene()
	{
#ifdef STORMANCER_LOG_CLIENT
		ILogger::instance()->log(LogLevel::Trace, "Scene::~Scene", "deleting the scene...", "");
#endif

		_onDelete();

		disconnect(true);

		for (auto weakSub : _subscriptions)
		{
			auto sub = weakSub.get_weak().lock();
			if (sub)
			{
				sub->unsubscribe();
			}
		}
		_subscriptions.clear();

		if (_host)
		{
			delete _host;
		}

#ifdef STORMANCER_LOG_CLIENT
		ILogger::instance()->log(LogLevel::Trace, "Scene::~Scene", "scene deleted", "");
#endif
	}

	const std::string Scene::getHostMetadata(const std::string key)
	{
		return (mapContains(_metadata, key) ? _metadata[key] : std::string());
	}

	byte Scene::handle()
	{
		return _handle;
	}

	const std::string Scene::id()
	{
		return _id;
	}

	ConnectionState Scene::GetCurrentConnectionState()
	{
		return _connectionState;
	}

	rxcpp::observable<ConnectionState> Scene::GetConnectionStateChangedObservable()
	{
		return _connectionStateObservable.get_observable();
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

	void Scene::addRoute(const std::string routeName2, std::function<void(Packetisp_ptr)> handler, const stringMap* metadata2)
	{
		try
		{
			std::string routeName = routeName2;

			stringMap metadata;
			if (metadata2)
			{
				metadata = *metadata2;
			}

			if (routeName.size() == 0 || routeName[0] == '@')
			{
				throw std::invalid_argument("A route cannot be empty or start with the '@' character.");
			}

			if (GetCurrentConnectionState() == ConnectionState::Connected)
			{
				throw std::runtime_error("You cannot register handles once the scene is connected.");
			}

			if (!mapContains(_localRoutesMap, routeName))
			{
				_localRoutesMap[routeName] = Route_ptr(new Route(this, routeName, metadata));
			}

			auto subscription = onMessage(routeName2).subscribe(handler);
			_subscriptions.push_back(subscription);

		}
		catch (const std::exception&)
		{
			throw;
		}
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

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(const std::string routeName2)
	{
		std::string routeName = routeName2;
		if (GetCurrentConnectionState() == ConnectionState::Connected)
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

	void Scene::sendPacket(const std::string routeName2, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		try
		{
			if (routeName2.length() == 0)
			{
				throw std::invalid_argument("routeName is empty.");
			}

			if (GetCurrentConnectionState() != ConnectionState::Connected)
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
		catch (const std::exception&)
		{
			throw;
		}

	}

	pplx::task<void> Scene::connect()
	{

		try
		{
			if (GetCurrentConnectionState() == ConnectionState::Disconnected && _client)
			{
				_connectTask = _client->connectToScene(this->shared_from_this(), _token, mapValues(_localRoutesMap));
			}
		}
		catch (const std::exception& ex)
		{

			return pplx::task_from_exception<void>(ex);
		}

		return _connectTask.then([](pplx::task<void> t) {
			try
			{
				t.wait();

			}
			catch (const std::exception& ex)
			{
				ILogger::instance()->log(LogLevel::Error, "scene.connect", "Failed to connect the scene", ex.what());
				throw;
			}

		});
	}

	pplx::task<void> Scene::disconnect(bool immediate)
	{
		if (GetCurrentConnectionState() == ConnectionState::Connected && _client)
		{
			_disconnectTask = _client->disconnect(this, _handle, immediate).then([this](pplx::task<void> t) {

				this->setConnectionState(ConnectionState::Disconnected);
			});
		}
		else if(GetCurrentConnectionState() != ConnectionState::Disconnecting)
		{
			_disconnectTask = pplx::task_from_result();
		}

		return _disconnectTask;
	}

	DependencyResolver* Scene::dependencyResolver() const
	{
		return _dependencyResolver;
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
		_onPacketReceived(packet);

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

	std::vector<IScenePeer*> Scene::remotePeers()
	{
		std::vector<IScenePeer*> container;
		container.push_back(host());
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

	bool Scene::isHost() const
	{
		return _isHost;
	}

	Action<Packet_ptr>::TIterator Scene::onPacketReceived(std::function<void(Packet_ptr)> callback)
	{
		return _onPacketReceived.push_back(callback);
	}

	Action<Packet_ptr>& Scene::packetReceivedAction()
	{
		return _onPacketReceived;
	}

	void Scene::setConnectionState(ConnectionState state)
	{
		if (_connectionState != state)
		{
			_connectionStateObservable.get_subscriber().on_next(state);
		}
		if (_connectionState == ConnectionState::Disconnected)
		{
			_connectionStateObservable.get_subscriber().on_completed();
		}
	}
};
