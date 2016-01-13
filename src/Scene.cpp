#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, Client* client, std::string id, std::string token, SceneInfosDto dto, DependencyResolver* parentDependencyResolver, std::function<void()> onDelete)
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

		_peerConnectionStateEraseIterator = _peer->onConnectionStateChanged([this](ConnectionState connectionState) {
			if (connectionState == ConnectionState::Disconnected)
			{
				setConnectionState(ConnectionState::Disconnected);
			}
		});
	}

	Scene::~Scene()
	{
		_peer->connectionStateChangedAction().erase(_peerConnectionStateEraseIterator);

		_onDelete();

		disconnect(true);

		for (auto weakSub : _subscriptions)
		{
			auto sub = weakSub.lock();
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
	}
	
	const char* Scene::getHostMetadata(const char* key)
	{
		return (mapContains(_metadata, std::string(key)) ? _metadata[key].c_str() : nullptr);
	}

	byte Scene::handle()
	{
		return _handle;
	}

	const char* Scene::id()
	{
		return _id.c_str();
	}

	ConnectionState Scene::connectionState()
	{
		return _connectionState;
	}

	Action<ConnectionState>& Scene::onConnectionStateChangedAction()
	{
		return _onConnectionStateChanged;
	}

	Action<ConnectionState>::TIterator Scene::onConnectionStateChanged(std::function<void(ConnectionState)> callback)
	{
		return _onConnectionStateChanged.push_back(callback);
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

	Result<>* Scene::addRoute(const char* routeName2, std::function<void(Packetisp_ptr)> handler, const stringMap* metadata2)
	{
		auto result = new Result<>();
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

			if (_connectionState == ConnectionState::Connected)
			{
				throw std::runtime_error("You cannot register handles once the scene is connected.");
			}

			if (!mapContains(_localRoutesMap, routeName))
			{
				_localRoutesMap[routeName] = Route_ptr(new Route(this, routeName, metadata));
			}

			auto subscription = onMessage(routeName2)->subscribe(handler);
			_subscriptions.push_back(subscription);
			result->set();
		}
		catch (const std::exception& ex)
		{
			result->setError(1, ex.what());
		}
		return result;
	}

	IObservable<Packetisp_ptr>* Scene::onMessage(Route_ptr route)
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
		IObservable<Packetisp_ptr>* res = new Observable<Packetisp_ptr>(observable.as_dynamic());
		return res;
	}

	IObservable<Packetisp_ptr>* Scene::onMessage(const char* routeName2)
	{
		std::string routeName = routeName2;
		if (_connectionState == ConnectionState::Connected)
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

	Result<>* Scene::sendPacket(const char* routeName2, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		auto result = new Result<>();
		try
		{
			if (!std::strlen(routeName2))
			{
				throw std::invalid_argument("routeName is empty.");
			}

			if (_connectionState != ConnectionState::Connected)
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
			result->set();
		}
		catch (const std::exception& ex)
		{
			result->setError(1, ex.what());
		}
		return result;
	}

	pplx::task<Result<>*> Scene::connect()
	{
		auto result = new Result<>();
		
		try
		{
			if (_connectionState == ConnectionState::Disconnected && _client)
			{
				_connectTask = _client->connectToScene(this, _token, mapValues(_localRoutesMap));
			}
		}
		catch (const std::exception& ex)
		{
			result->setError(1, ex.what());
			return taskCompleted(result);
		}

		return _connectTask.then([result](pplx::task<void> t) {
			try
			{
				t.wait();
				result->set();
			}
			catch (const std::exception& ex)
			{
				ILogger::instance()->log(LogLevel::Error, "scene.connect", "Failed to connect the scene", ex.what());
				result->setError(1, ex.what());
			}
			return result;
		});
	}

	pplx::task<Result<>*> Scene::disconnect(bool immediate)
	{
		if (_connectionState == ConnectionState::Connected && _client)
		{
			_disconnectTask = _client->disconnect(this, _handle, immediate);
		}
		else
		{
			_disconnectTask = taskCompleted();
		}

		return _disconnectTask.then([](pplx::task<void> t) {
			auto result = new Result<>();
			try
			{
				t.wait();
				result->set();
			}
			catch (const std::exception& ex)
			{
				result->setError(1, ex.what());
			}
			return result;
		});
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
			_connectionState = state;
			_onConnectionStateChanged(state);
		}
	}
};
