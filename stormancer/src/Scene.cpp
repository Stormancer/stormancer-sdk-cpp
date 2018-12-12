#include "stormancer/stdafx.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/ScenePeer.h"
#include "stormancer/Client.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/ChannelUidStore.h"

namespace Stormancer
{
	Scene_Impl::Scene_Impl(std::weak_ptr<IConnection> connection, std::weak_ptr<Client> client, const SceneAddress& address, const std::string& token, const SceneInfosDto& dto, std::weak_ptr<DependencyResolver> parentDependencyResolver, std::vector<IPlugin*>& plugins)
		: _dependencyResolver(std::make_shared<DependencyResolver>(parentDependencyResolver))
		, _peer(connection)
		, _token(token)
		, _metadata(dto.Metadata)
		, _address(address)
		, _client(client)
		, _logger(_dependencyResolver->resolve<ILogger>())
		, _plugins(plugins)
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = std::make_shared<Route>(routeDto.Name, routeDto.Handle, MessageOriginFilter::Host, routeDto.Metadata);
		}


	}
	void Scene_Impl::initialize()
	{
		_host = std::make_shared<ScenePeer>(_peer, _handle, _remoteRoutesMap, this->shared_from_this());
		std::weak_ptr<Scene_Impl> wThat = this->shared_from_this();
		auto onNext = [wThat](ConnectionState state) {
			if (auto that = wThat.lock())
			{
				that->_connectionState = state;
			}

		};

		auto onError = [wThat](std::exception_ptr exptr) {
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				if (auto that = wThat.lock())
				{
					that->_logger->log(LogLevel::Error, "Scene", "Connection state change failed", ex.what());
				}
			}
		};

		_sceneConnectionStateObservable.get_observable().subscribe(onNext, onError);
		_hostConnectionStateSubscription = _peer.lock()->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state) {
			if (auto that = wThat.lock())
			{
				auto sceneState = that->getCurrentConnectionState();
				auto actionDispatcher = that->dependencyResolver()->resolve<IActionDispatcher>();
				// We check the connection is disconnecting, and the scene is not already disconnecting or disconnected
				if (state == ConnectionState::Disconnecting && sceneState != ConnectionState::Disconnecting && sceneState != ConnectionState::Disconnected)
				{
					// We are disconnecting the scene
					actionDispatcher->post([wThat, state]()
					{
						if (auto that = wThat.lock())
						{
							that->setConnectionState(ConnectionState(ConnectionState::Disconnecting, state.reason));
						}
					});


				}
				// We check the connection is disconnected, and the scene is not already disconnected
				else if (state == ConnectionState::Disconnected && sceneState != ConnectionState::Disconnected)
				{
					// We ensure the scene is disconnecting
					if (sceneState != ConnectionState::Disconnecting)
					{
						actionDispatcher->post([wThat, state]()
						{
							if (auto that = wThat.lock())
							{
								that->setConnectionState(ConnectionState(ConnectionState::Disconnecting, state.reason));
							}
						});
					}

					// We disconnect the scene
					actionDispatcher->post([wThat, state]()
					{
						if (auto that = wThat.lock())
						{
							that->setConnectionState(ConnectionState(ConnectionState::Disconnected, state.reason));
						}
					});

				}
			}
		});

		for (auto plugin : this->_plugins)
		{
			plugin->registerSceneDependencies(this->shared_from_this());
		}
		for (auto plugin : this->_plugins)
		{
			plugin->sceneCreated(this->shared_from_this());
		}
	}
	Scene_Impl::~Scene_Impl()
	{
		if (_hostConnectionStateSubscription.is_subscribed())
		{
			_hostConnectionStateSubscription.unsubscribe();
		}
		_logger->log(LogLevel::Trace, "Scene", "destroying scene", _address.toUri());

		auto logger = _logger;
		disconnect().then([logger](pplx::task<void> t) {
			try
			{
				t.wait();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Trace, "Client", "Ignore scene disconnection failure", ex.what());
			}
		});

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

		_logger->log(LogLevel::Trace, "Scene", "Scene destroyed.", _address.toUri());
	}



	std::string Scene_Impl::getHostMetadata(const std::string& key) const
	{
		if (mapContains(_metadata, key))
		{
			return _metadata.at(key);
		}
		return std::string();
	}

	byte Scene_Impl::handle() const
	{
		return _handle;
	}
	SceneAddress Scene_Impl::address() const
	{
		return _address;
	}
	std::string Scene_Impl::id() const
	{
		return _address.sceneId;
	}

	ConnectionState Scene_Impl::getCurrentConnectionState() const
	{
		return _connectionState;
	}

	rxcpp::observable<ConnectionState> Scene_Impl::getConnectionStateChangedObservable() const
	{
		return _sceneConnectionStateObservable.get_observable();
	}

	std::weak_ptr<IConnection> Scene_Impl::hostConnection() const
	{
		return _peer;
	}

	std::vector<Route_ptr> Scene_Impl::localRoutes() const
	{
		return mapValues(_localRoutesMap);
	}

	std::vector<Route_ptr> Scene_Impl::remoteRoutes() const
	{
		return mapValues(_remoteRoutesMap);
	}

	void Scene_Impl::addRoute(const std::string& routeName, std::function<void(Packetisp_ptr)> handler, MessageOriginFilter filter, const std::map<std::string, std::string>& metadata)
	{
		auto observable = onMessage(routeName, filter, metadata);

		auto onError = [=](std::exception_ptr exptr)
		{
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "Scene", "Error reading message on route '" + routeName + "'", ex.what());
			}
		};

		auto subscription = observable.subscribe(handler, onError);
		_subscriptions.push_back(subscription);
	}

	rxcpp::observable<Packetisp_ptr> Scene_Impl::onMessage(const std::string& routeName, MessageOriginFilter filter, const std::map<std::string, std::string>& metadata)
	{
		std::lock_guard<std::mutex> lg(_connectMutex);

		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("The client is deleted.");
		}

		if (_connectionState != ConnectionState::Disconnected)
		{
			throw std::runtime_error("A route cannot be added to a scene already connected.");
		}

		if (routeName.size() == 0 || routeName[0] == '@')
		{
			throw std::invalid_argument("A route cannot be empty or start with the '@' character.");
		}

		if (mapContains(_localRoutesMap, routeName))
		{
			throw std::runtime_error("A route already exists for this route name.");
		}

		auto route = std::make_shared<Route>(routeName, (uint16)0, filter, metadata);
		_localRoutesMap[routeName] = route;
		return onMessage(route);
	}

	rxcpp::observable<Packetisp_ptr> Scene_Impl::onMessage(Route_ptr route)
	{
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([=](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto handler = std::function<void(Packet_ptr)>([=](Packet_ptr p) {
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
						((!isOriginHost) && ((int)route->filter() & (int)MessageOriginFilter::Peer))) //Filter packet
					{
						Packetisp_ptr packet(new Packet<IScenePeer>(origin, p->stream, p->metadata));
						subscriber.on_next(packet);
					}
				}
			});
			route->handlers.push_back(handler);
			auto it = route->handlers.end();
			it--;
			subscriber.add([=]() {
				route->handlers.erase(it);
			});
		});

		return observable.as_dynamic();
	}

	void Scene_Impl::send(const PeerFilter&, const std::string& routeName, const Writer& writer, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("Client deleted");
		}

		auto peer = _peer.lock();
		if (!peer)
		{
			throw std::runtime_error("Peer deleted");
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
		int channelUid = 1;
		if (channelIdentifier.empty())
		{
			std::stringstream ss;
			ss << "Scene_" << id() << "_" << routeName;
			channelUid = peer->dependencyResolver()->resolve<ChannelUidStore>()->getChannelUid(ss.str());
		}
		else
		{
			channelUid = peer->dependencyResolver()->resolve<ChannelUidStore>()->getChannelUid(channelIdentifier);
		}
		auto writer2 = [=, &writer](obytestream* stream) {
			(*stream) << _handle;
			(*stream) << route->handle();
			if (writer)
			{
				writer(stream);
			}
		};
		TransformMetadata transformMetadata{ shared_from_this() };
		peer->send(writer2, channelUid, priority, reliability, transformMetadata);
	}

	void Scene_Impl::send(const std::string& routeName, const Writer& writer, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		send(MatchSceneHost(), routeName, writer, priority, reliability, channelIdentifier);
	}

	pplx::task<void> Scene_Impl::connect(pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lg(_connectMutex);

		if (_connectionState == ConnectionState::Disconnected)
		{
			auto client = _client.lock();
			if (client)
			{
				_connectTask = client->connectToScene(this->shared_from_this(), _token, mapValues(_localRoutesMap), ct);
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

	pplx::task<void> Scene_Impl::disconnect()
	{
		try
		{
			_logger->log(LogLevel::Trace, "Scene", "Scene disconnecting");

			std::lock_guard<std::mutex> lg(_disconnectMutex);
			_disconnectTask = pplx::task_from_result();
			if (_connectionState == ConnectionState::Connected)
			{
				auto logger = _logger;
				auto client = _client.lock();
				if (client)
				{
					_disconnectTask = client->disconnect(shared_from_this());
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
		}
		catch (std::exception& ex)
		{
			_disconnectTask.then([](pplx::task<void> t) {
				try
				{
					t.get();
				}
				catch (std::exception&)
				{
				}
			});
			_disconnectTask = pplx::task_from_exception<void>(ex);
		}

		return _disconnectTask;
	}


	std::shared_ptr<DependencyResolver> Scene_Impl::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	void Scene_Impl::completeConnectionInitialization(ConnectionResult& cr)
	{
		_handle = cr.SceneHandle;

		for (auto pair : _localRoutesMap)
		{
			pair.second->setHandle(cr.RouteMappings[pair.first]);
			_handlers[pair.second->handle()] = pair.second->handlers;
		}
	}

	void Scene_Impl::handleMessage(Packet_ptr packet)
	{
		_onPacketReceived(packet);

		uint16 routeHandle;
		*packet->stream >> routeHandle;


		if (mapContains(_handlers, routeHandle))
		{
			Route_ptr route;
			for (auto it = _localRoutesMap.begin(); it != _localRoutesMap.end(); ++it)
			{
				if (it->second->handle() == routeHandle)
				{
					route = it->second;
					break;
				}

			}
			if (packet->connection->id() == _host->id() && !((int)route->filter() & (int)Stormancer::MessageOriginFilter::Host))
			{
				return; // The route doesn't accept messages from the scene host.
			}

			if (packet->connection->id() != _host->id() && !((int)route->filter() & (int)Stormancer::MessageOriginFilter::Peer))
			{
				return; // The route doesn't accept messages from the scene host.
			}

			packet->metadata["routeId"] = route->name();
			auto observers = _handlers[routeHandle];
			for (auto f : observers)
			{
				f(packet);
			}
		}
	}

	std::vector<IScenePeer*> Scene_Impl::remotePeers() const
	{
		std::vector<IScenePeer*> container;
		container.push_back(host());
		return container;
	}

	IScenePeer* Scene_Impl::host() const
	{
		return _host.get();
	}

	bool Scene_Impl::isHost() const
	{
		return _isHost;
	}

	std::unordered_map<uint64, std::shared_ptr<IScenePeer>> Scene_Impl::connectedPeers() const
	{
		return _connectedPeers;
	}

	rxcpp::observable<P2PConnectionStateChangedArgs> Scene_Impl::p2pConnectionStateChanged() const
	{
		return _p2pConnectionStateChangedObservable.get_observable();
	}

	std::shared_ptr<P2PTunnel> Scene_Impl::registerP2PServer(const std::string & p2pServerId)
	{
		auto p2p = dependencyResolver()->resolve<P2PService>();
		return p2p->registerP2PServer(this->id() + "." + p2pServerId);
	}

	pplx::task<std::shared_ptr<IP2PScenePeer>> Scene_Impl::openP2PConnection(const std::string & p2pToken, pplx::cancellation_token ct)
	{
		auto p2pService = dependencyResolver()->resolve<P2PService>();
		auto wScene = STRM_WEAK_FROM_THIS();
		auto dispatcher = dependencyResolver()->resolve<IActionDispatcher>();
		auto options = pplx::task_options(dispatcher);
		options.set_cancellation_token(ct);
		return p2pService->openP2PConnection(_peer.lock(),p2pToken, ct)
			.then([wScene, p2pService](std::shared_ptr<IConnection> connection)
		{
			auto scene = LockOrThrow(wScene);
			return std::static_pointer_cast<IP2PScenePeer>(std::make_shared<P2PScenePeer>(scene, connection, p2pService, P2PConnectToSceneMessage()));
		}, options);
		//	auto c = t.get();
		//	P2PConnectToSceneMessage message;
		//	return this->sendSystemRequest<P2PConnectToSceneMessage, P2PConnectToSceneMessage>(c,(byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, message).then([c](P2PConnectToSceneMessage m) {
		//		return std::make_tuple(c, m);
		//	});
		//}).then([=](pplx::task<std::tuple<std::shared_ptr<IConnection>,P2PConnectToSceneMessage>> t) {
		//	auto tuple = t.get();
		//	return std::make_shared<P2PScenePeer>(this,std::get<0>(tuple),p2p,std::get<1>(tuple));
		//});
	}

	const std::map<std::string, std::string>& Scene_Impl::getSceneMetadata()
	{
		return _metadata;
	}

	Action2<Packet_ptr> Scene_Impl::onPacketReceived()
	{
		return _onPacketReceived;
	}

	

	void Scene_Impl::setConnectionState(ConnectionState state)
	{
		if (_connectionState != state && !_connectionStateObservableCompleted)
		{
			
			_connectionState = state;
			// on_next and on_completed handlers might delete this. Do not use this at all after calling the handlers.
			_connectionStateObservableCompleted = (state == ConnectionState::Disconnected);
			auto subscriber = _sceneConnectionStateObservable.get_subscriber();

			switch (state.state)
			{
			case ConnectionState::Connecting:
				for (auto plugin : this->_plugins)
				{
					plugin->sceneConnecting(this->shared_from_this());
				}
				break;
			case ConnectionState::Connected:
				for (auto plugin : this->_plugins)
				{
					plugin->sceneConnected(this->shared_from_this());
				}
				break;
			case ConnectionState::Disconnecting:
				for (auto plugin : this->_plugins)
				{
					plugin->sceneDisconnecting(this->shared_from_this());
				}
				break;
			case ConnectionState::Disconnected:
				for (auto plugin : this->_plugins)
				{
					plugin->sceneDisconnected(this->shared_from_this());
				}
				break;
			default:
				break;
			}

			subscriber.on_next(state);
			if (state == ConnectionState::Disconnected)
			{
				subscriber.on_completed();
			}
		}

	}

	pplx::task<std::shared_ptr<P2PScenePeer>> Scene_Impl::createScenePeerFromP2PConnection(std::shared_ptr<IConnection> /*connection*/, pplx::cancellation_token /*ct*/)
	{
		throw std::runtime_error("Not implemented");
	}
};
