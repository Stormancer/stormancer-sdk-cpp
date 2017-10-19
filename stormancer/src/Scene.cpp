#include "stdafx.h"
#include "Scene.h"
#include "ScenePeer.h"
#include "Client.h"

namespace Stormancer
{
	Scene::Scene(IConnection* connection, std::weak_ptr<Client> client, const std::string& id, const std::string& token, const SceneInfosDto& dto, std::weak_ptr<DependencyResolver> parentDependencyResolver)
		: _id(id)
		, _peer(connection)
		, _token(token)
		, _client(client)
		, _metadata(dto.Metadata)
		, _dependencyResolver(std::make_shared<DependencyResolver>(parentDependencyResolver))
		, _logger(_dependencyResolver->resolve<ILogger>())
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = std::make_shared<Route>(routeDto.Name, routeDto.Handle, MessageOriginFilter::Host, routeDto.Metadata);
		}

		_connectionStateObservable.get_observable().subscribe([this](ConnectionState state) {
			_connectionState = state;
		});
	}

	Scene::~Scene()
	{
#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Scene", "deleting scene...", _id);
#endif
		auto& logger = _logger;
		disconnect(true).then([logger](pplx::task<void> t) {
			try
			{
				t.get();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Trace, "Scene", "Scene disconnection failed.", ex.what());
			}
		}).wait();

		for (auto weakSub : _subscriptions)
		{
			auto sub = weakSub.get_weak().lock();
			if (sub)
			{
				sub->unsubscribe();
			}
		}
		_subscriptions.clear();

		_localRoutesMap.clear();
		_remoteRoutesMap.clear();

#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Scene", "Scene deleted.", _id);
#endif
	}

	void Scene::initialize()
	{
		_host = std::make_shared<ScenePeer>(_peer, _handle, _remoteRoutesMap, this);
	}

	std::string Scene::getHostMetadata(const std::string& key) const
	{
		if (mapContains(_metadata, key))
		{
			return _metadata.at(key);
		}
		return std::string();
	}

	byte Scene::handle() const
	{
		return _handle;
	}

	std::string Scene::id() const
	{
		return _id;
	}

	ConnectionState Scene::getCurrentConnectionState() const
	{
		return _connectionState;
	}

	rxcpp::observable<ConnectionState> Scene::getConnectionStateChangedObservable() const
	{
		return _connectionStateObservable.get_observable();
	}

	IConnection* Scene::hostConnection() const
	{
		return _peer;
	}

	std::vector<Route_ptr> Scene::localRoutes() const
	{
		return mapValues(_localRoutesMap);
	}

	std::vector<Route_ptr> Scene::remoteRoutes() const
	{
		return mapValues(_remoteRoutesMap);
	}

	void Scene::addRoute(const std::string& routeName, std::function<void(Packetisp_ptr)> handler, MessageOriginFilter filter, const std::map<std::string, std::string>& metadata)
	{
		auto subscription = onMessage(routeName, filter, metadata)
			.subscribe(
				handler,
				// On error
				[this, routeName](std::exception_ptr eptr)
				{
					try
					{
						std::rethrow_exception(eptr);
					}
					catch (std::exception& e)
					{
						_logger->log(LogLevel::Error, "Scene", "Error reading message on route " + routeName, e.what());
					}
				}
		);
		_subscriptions.push_back(subscription);
	}

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(const std::string& routeName, MessageOriginFilter filter, const std::map<std::string, std::string>& metadata)
	{
		std::lock_guard<std::mutex> lg(_connectMutex);

		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("The client is deleted.");
		}

		if (_connectionState != ConnectionState::Disconnected)
		{
			throw std::runtime_error("The scene is not disconnected.");
		}

		if (routeName.size() == 0 || routeName[0] == '@')
		{
			throw std::invalid_argument("A route cannot be empty or start with the '@' character.");
		}

		if (mapContains(_localRoutesMap, routeName))
		{
			throw std::runtime_error("A route already exists for this route name.");
		}

		auto route = std::make_shared<Route>(routeName, 0, filter, metadata);
		_localRoutesMap[routeName] = route;
		return onMessage(route);
	}

	rxcpp::observable<Packetisp_ptr> Scene::onMessage(Route_ptr route)
	{
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([this, route](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto handler = std::function<void(Packet_ptr)>([this, subscriber, route](Packet_ptr p) {
				std::shared_ptr<IScenePeer> origin = nullptr;
				bool isOriginHost = false;
				if (p->connection->id() == host()->id())
				{
					isOriginHost = true;
					origin = _host;
				}
				else
				{
					origin = _connectedPeers[p->connection->id()];
				}
				if (origin)
				{
					if ((isOriginHost && ((int)route->filter() & (int)MessageOriginFilter::Host)) ||
						(!isOriginHost) && ((int)route->filter() & (int)MessageOriginFilter::Peer)) //Filter packet
					{
						Packetisp_ptr packet(new Packet<IScenePeer>(origin, p->stream, p->metadata));
						subscriber.on_next(packet);
					}
				}
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

	void Scene::send(const PeerFilter&, const std::string& routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("The client is deleted");
		}

		if (_connectionState != ConnectionState::Connected)
		{
			throw std::runtime_error("The scene is not connected");
		}

		if (routeName.length() == 0)
		{
			throw std::invalid_argument("The Route name is invalid");
		}

		if (!mapContains(_remoteRoutesMap, routeName))
		{
			throw std::invalid_argument(std::string() + "The route '" + routeName + "' doesn't exist on the scene");
		}

		Route_ptr route = _remoteRoutesMap[routeName];
		_peer->sendToScene(_handle, route->handle(), writer, priority, reliability);
	}

	void Scene::send(const std::string& routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		send(MatchSceneHost(), routeName, writer, priority, reliability);
	}

	void Scene::sendPacket(const std::string& routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		send(MatchSceneHost(), routeName, writer, priority, reliability);
	}

	pplx::task<void> Scene::connect()
	{
		std::lock_guard<std::mutex> lg(_connectMutex);

		if (_connectionState == ConnectionState::Disconnected)
		{
			auto client = _client.lock();
			if (client)
			{
				_connectTask = client->connectToScene(_id, _token, mapValues(_localRoutesMap));
			}
			else
			{
				_connectTask = pplx::task_from_exception<void>(std::runtime_error("Client is deleted."));
			}
		}
		else if (_connectionState == ConnectionState::Disconnecting)
		{
			_connectTask = pplx::task_from_exception<void>(std::runtime_error("Scene is disconnecting."));
		}

		return _connectTask;
	}

	pplx::task<void> Scene::disconnect(bool immediate)
	{
#ifdef STORMANCER_LOG_CLIENT
		_logger->log(LogLevel::Trace, "Scene", std::string() + "Scene disconnecting, immediate=" + (immediate ? "true" : "false"));
#endif

		std::lock_guard<std::mutex> lg(_disconnectMutex);

		if (_connectionState == ConnectionState::Connected)
		{
			auto client = _client.lock();
			if (client)
			{
				_disconnectTask = client->disconnect(this);
			}
			else
			{
				_logger->log(LogLevel::Warn, "Scene", "Client is invalid.");
				_disconnectTask = pplx::task_from_exception<void>(std::runtime_error("Client is invalid."));
			}
		}
		else if (_connectionState == ConnectionState::Connecting)
		{
			_disconnectTask = pplx::task_from_exception<void>(std::runtime_error("Scene is connecting."));
		}
		else if (_connectionState == ConnectionState::Disconnecting)
		{
			_disconnectTask = pplx::task_from_result();
		}
		else if (_connectionState == ConnectionState::Disconnected)
		{
			_disconnectTask = pplx::task_from_result();
		}

		if (immediate)
		{
			// if immediate, we don't propage the exception
			return _disconnectTask.then([this](pplx::task<void> t) {
				try
				{
					t.wait();
				}
				catch (const std::exception& ex)
				{
					_logger->log(LogLevel::Warn, "Scene", "Scene disconnection failed.", ex.what());
				}
			});
		}
		else
		{
			return _disconnectTask;
		}
	}

	DependencyResolver* Scene::dependencyResolver() const
	{
		return _dependencyResolver.get();
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

		packet->metadata["routeId"] = std::to_string(routeId);

		if (mapContains(_handlers, routeId))
		{
			auto observers = _handlers[routeId];
			for (auto f : observers)
			{
				f(packet);
			}
		}
	}

	std::vector<IScenePeer*> Scene::remotePeers() const
	{
		std::vector<IScenePeer*> container;
		container.push_back(host());
		return container;
	}

	IScenePeer* Scene::host() const
	{
		return _host.get();
	}

	bool Scene::isHost() const
	{
		return _isHost;
	}

	std::unordered_map<uint64, std::shared_ptr<IScenePeer>> Scene::connectedPeers() const
	{
		return _connectedPeers;
	}

	rxcpp::observable<P2PConnectionStateChangedArgs> Scene::p2pConnectionStateChanged() const
	{
		return _p2pConnectionStateChangedObservable.get_observable();
	}

	std::shared_ptr<P2PTunnel> Scene::registerP2PServer(const std::string & p2pServerId)
	{
		auto p2p = dependencyResolver()->resolve<P2PService>();
		return p2p->registerP2PServer(this->id()+"."+p2pServerId);

	}
	pplx::task<std::shared_ptr<P2PScenePeer>> Scene::openP2PConnection(const std::string & token)
	{
		auto p2p = dependencyResolver()->resolve<P2PService>();
		return p2p->openP2PConnection(token).then([this, p2p](pplx::task < std::shared_ptr<IConnection>> t) {

			return std::make_shared<P2PScenePeer>(this, t.get(), p2p);
		});
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
		if (_connectionState != state && !_connectionStateObservableCompleted)
		{
			// on_next and on_completed handlers might delete this. Do not use this at all after calling the handlers.
			_connectionStateObservableCompleted = (state == ConnectionState::Disconnected);
			auto subscriber = _connectionStateObservable.get_subscriber();

			subscriber.on_next(state);
			if (state == ConnectionState::Disconnected)
			{
				subscriber.on_completed();
			}
		}
	}
};
