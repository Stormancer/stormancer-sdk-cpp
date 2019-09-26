#include "stormancer/stdafx.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/ScenePeer.h"
#include "stormancer/Client.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/ChannelUidStore.h"
#include "stormancer/ConnectToSceneMsg.h"
#include "stormancer/P2P/P2PService.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	Scene_Impl::Scene_Impl(std::weak_ptr<IConnection> connection, std::weak_ptr<Client> client, const SceneAddress& address, const std::string& token, const SceneInfosDto& dto, DependencyScope& parentScope, std::vector<IPlugin*>& plugins)
		: _peer(connection)
		, _token(token)
		, _metadata(dto.Metadata)
		, _address(address)
		, _client(client)
		, _logger(parentScope.resolve<ILogger>())
		, _plugins(plugins)
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutes[routeDto.Name] = std::make_shared<Route>(routeDto.Name, routeDto.Handle, MessageOriginFilter::Host, routeDto.Metadata);
		}
	}

	void Scene_Impl::initialize(DependencyScope& parentScope)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		std::weak_ptr<Scene_Impl> wThat = this->shared_from_this();
		auto onNext = [wThat](ConnectionState state)
		{
			if (auto that = wThat.lock())
			{
				that->_connectionState = state;
			}
		};

		auto onError = [wThat](std::exception_ptr exptr)
		{
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

		auto peer = _peer.lock();
		if (!peer)
		{
			throw std::runtime_error("Peer destroyed");
		}

		_sceneConnectionStateObservable.get_observable().subscribe(onNext, onError);
		_hostConnectionStateSubscription = peer->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state)
			{
				if (auto that = wThat.lock())
				{
					auto sceneState = that->getCurrentConnectionState();
					pplx::task<void> setConnectionStateTask = pplx::task_from_result();
					// We check the connection is disconnecting, and the scene is not already disconnecting or disconnected
					if (state == ConnectionState::Disconnecting && sceneState != ConnectionState::Disconnecting && sceneState != ConnectionState::Disconnected)
					{
						// We are disconnecting the scene
						setConnectionStateTask = that->setConnectionState(ConnectionState(ConnectionState::Disconnecting, state.reason));
					}
					// We check the connection is disconnected, and the scene is not already disconnected
					else if (state == ConnectionState::Disconnected && sceneState != ConnectionState::Disconnected)
					{
						// We ensure the scene is disconnecting
						if (sceneState != ConnectionState::Disconnecting)
						{
							setConnectionStateTask = that->setConnectionState(ConnectionState(ConnectionState::Disconnecting, state.reason));
						}

						setConnectionStateTask = setConnectionStateTask.then([wThat, state]
							{
								if (auto that = wThat.lock())
								{
									return that->setConnectionState(ConnectionState(ConnectionState::Disconnected, state.reason));
								}
								return pplx::task_from_result();
							}, pplx::get_ambient_scheduler());
					}
					setConnectionStateTask.then([wThat](pplx::task<void> task)
						{
							try
							{
								task.get();
							}
							catch (const std::exception& ex)
							{
								if (auto that = wThat.lock())
								{
									that->_logger->log(LogLevel::Warn, "Scene::hostConnectionStateSubscription", "An error occurred while running ConnectionState event handlers", ex.what());
								}
							}
						});
				}
			}, [](std::exception_ptr exptr) // on_error callback, added because the on_next above might throw
			{
				try
				{
					std::rethrow_exception(exptr);
				}
				catch (const std::exception&)
				{
					// Most likely invalidated DependencyScope because of dead Client...
				}
			});

		_scope = parentScope.beginLifetimeScope([this](ContainerBuilder& builder)
			{
				auto weakThis = STORM_WEAK_FROM_THIS();
				builder.registerDependency<Scene_Impl>([weakThis](const DependencyScope&) { return weakThis.lock(); }).as<Scene>().instancePerRequest();
				for (auto plugin : this->_plugins)
				{
					plugin->registerSceneDependencies(builder, this->shared_from_this());
				}
			});
		_dispatcher = _scope.resolve<IActionDispatcher>();
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

		for (auto weakSub : _subscriptions)
		{
			auto sub = weakSub.get_weak().lock();
			if (sub)
			{
				sub->unsubscribe();
			}
		}
		_subscriptions.clear();

		_localRoutes.clear();
		_remoteRoutes.clear();

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
		// This needs a lock because the client's destructor checks getCurrentConnectionState() != Disconnected before calling setConnectionState().
		// If the check is performed while the connectionState is being set to Disconnected asynchronously, without the mutex, it could be false before the plugin handlers have been called.
		// In this case, the client could destroy the plugins while the async operation of calling the handlers is taking place.
		// Adding this lock prevents this possibility.
		std::lock_guard<std::mutex> lg(_connectionStateMutex);

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
		return mapValues(_localRoutes);
	}

	std::vector<Route_ptr> Scene_Impl::remoteRoutes() const
	{
		return mapValues(_remoteRoutes);
	}

	void Scene_Impl::addRoute(const std::string& routeName, std::function<void(Packetisp_ptr)> handler, MessageOriginFilter filter, const std::unordered_map<std::string, std::string>& metadata)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		auto observable = onMessage(routeName, filter, metadata);

		auto logger = _logger;
		auto onError = [logger, routeName](std::exception_ptr exptr)
		{
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Error, "Scene", "Error reading message on route '" + routeName + "'", ex.what());
			}
		};

		auto subscription = observable.subscribe(handler, onError);
		_subscriptions.push_back(subscription);
	}

	rxcpp::observable<Packetisp_ptr> Scene_Impl::onMessage(const std::string& routeName, MessageOriginFilter filter, const std::unordered_map<std::string, std::string>& metadata)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("The client is deleted.");
		}

		if (_connectionState != ConnectionState::Disconnected && _connectionState != ConnectionState::Connecting)
		{
			throw std::runtime_error("A route cannot be added to a scene already connected.");
		}

		if (routeName.size() == 0 || routeName[0] == '@')
		{
			throw std::invalid_argument("A route cannot be empty or start with the '@' character.");
		}

		if (mapContains(_localRoutes, routeName))
		{
			throw std::runtime_error("A route already exists for this route name.");
		}

		auto route = std::make_shared<Route>(routeName, (uint16)0, filter, metadata);
		_localRoutes[routeName] = route;
		return onMessage(route);
	}

	rxcpp::observable<Packetisp_ptr> Scene_Impl::onMessage(Route_ptr route)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		auto wScene = STORM_WEAK_FROM_THIS();
		auto logger = _logger;
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([wScene, route, logger](rxcpp::subscriber<Packetisp_ptr> subscriber)
			{
				auto handler = std::function<void(Packet_ptr)>([wScene, route, subscriber, logger](Packet_ptr p)
					{
						auto scene = LockOrThrow(wScene);

						std::shared_ptr<IScenePeer> origin = nullptr;
						bool isOriginHost = false;
						if (p->connection->sessionId() == scene->host()->id())
						{
							isOriginHost = true;
							origin = scene->_host;
						}
						else
						{
							auto originIt = scene->_connectedPeers.find(p->connection->sessionId());
							if (originIt == scene->_connectedPeers.end())
							{
								logger->log(LogLevel::Warn, "Scene", "Peer (" + p->connection->sessionId() + ") not found on scene '" + scene->address().toUri() + "'");
								return;
							}
							else
							{
								origin = originIt->second;
							}
						}
						if (origin)
						{
							if ((isOriginHost && ((int)route->filter() & (int)MessageOriginFilter::Host)) ||
								((!isOriginHost) && ((int)route->filter() & (int)MessageOriginFilter::Peer))) // Filter packet
							{
								Packetisp_ptr packet(new Packet<IScenePeer>(origin, p->metadata));
								packet->stream.rdbuf()->pubsetbuf(p->stream.currentPtr(), p->stream.availableSize());
								packet->originalPacket = p; // The packetisp keeps alive the original packet
								subscriber.on_next(packet);
							}
						}
					});
				route->handlers.push_back(handler);
				auto handlerIt = route->handlers.end();
				handlerIt--;
				subscriber.add([route, handlerIt]()
					{
						route->handlers.erase(handlerIt);
					});
			});

		return observable.as_dynamic();
	}

	void Scene_Impl::send(const PeerFilter& peerFilter, const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		auto client = _client.lock();
		if (!client)
		{
			throw std::runtime_error("Client deleted");
		}

		if (_connectionState != ConnectionState::Connected)
		{
			throw std::runtime_error("The scene is not connected");
		}

		if (routeName.length() == 0)
		{
			throw std::invalid_argument("The Route name is invalid");
		}

		if (peerFilter == PeerFilterType::MatchSceneHost)
		{
			send(_host, routeName, streamWriter, priority, reliability, channelIdentifier);
		}
		else
		{
			if (peerFilter == PeerFilterType::MatchAllP2P)
			{
				std::lock_guard<std::recursive_mutex> lg(_mutex);
				for (auto scenePeerIt : _connectedPeers)
				{
					send(scenePeerIt.second, routeName, streamWriter, priority, reliability, channelIdentifier);
				}
			}
			else if (peerFilter == PeerFilterType::MatchPeers)
			{
				if (peerFilter.ids.size() == 0)
				{
					throw std::invalid_argument("peerFilter: A MatchPeers filter needs to have at least one Peer Session Id");
				}

				for (auto id : peerFilter.ids)
				{
					std::unordered_map<std::string, std::shared_ptr<IP2PScenePeer>>::iterator scenePeerIt;

					{
						std::lock_guard<std::recursive_mutex> lg(_mutex);
						scenePeerIt = _connectedPeers.find(id);

						if (scenePeerIt == _connectedPeers.end())
						{
							throw std::runtime_error("Peer not found");
						}
					}

					send(scenePeerIt->second, routeName, streamWriter, priority, reliability, channelIdentifier);
				}
			}
		}
	}

	void Scene_Impl::send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		send(PeerFilter::matchSceneHost(), routeName, streamWriter, priority, reliability, channelIdentifier);
	}

	void Scene_Impl::send(std::shared_ptr<IScenePeer> scenePeer, const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		auto remoteRoutes = scenePeer->routes();
		auto routeIt = remoteRoutes.find(routeName);
		if (routeIt == remoteRoutes.end())
		{
			throw std::invalid_argument(std::string() + "The route '" + routeName + "' doesn't exist in the ScenePeer");
		}

		auto& route = routeIt->second;
		auto connection = scenePeer->connection();
		auto channelUidStore = connection->dependencyResolver().resolve<ChannelUidStore>();

		int channelUid = 1;
		if (channelIdentifier.empty())
		{
			std::stringstream ss;
			ss << "Scene_" << id() << "_" << routeName;
			channelUid = channelUidStore->getChannelUid(ss.str());
		}
		else
		{
			channelUid = channelUidStore->getChannelUid(channelIdentifier);
		}

		auto sceneHandle = scenePeer->handle();
		auto routeHandle = route->handle();
		auto streamWriter2 = [sceneHandle, routeHandle, &streamWriter](obytestream& stream)
		{
			stream << sceneHandle;
			stream << routeHandle;
			if (streamWriter)
			{
				streamWriter(stream);
			}
		};
		TransformMetadata transformMetadata{ STORM_WEAK_FROM_THIS() };
		connection->send(streamWriter2, channelUid, priority, reliability, transformMetadata);
	}

	pplx::task<void> Scene_Impl::connect(pplx::cancellation_token ct)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		if (_connectionState == ConnectionState::Disconnected)
		{
			auto client = _client.lock();
			if (client)
			{
				auto linkedCt = create_linked_source(ct, _connectionCts.get_token()).get_token();
				_connectTask = client->connectToScene(this->shared_from_this(), _token, mapValues(_localRoutes), linkedCt);
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

	pplx::task<void> Scene_Impl::disconnect(pplx::cancellation_token ct)
	{
		_logger->log(LogLevel::Trace, "Scene", "Scene disconnecting");

		std::lock_guard<std::recursive_mutex> lg(_mutex);
		if (!_disconnectRequested)
		{
			_disconnectRequested = true;
			auto wScene = STORM_WEAK_FROM_THIS();
			_disconnectTask = disconnectAllPeers(ct)
				.then([ct, wScene]()
					{
						auto scene = LockOrThrow(wScene);
						return scene->disconnectFromHost(ct);
					})
				.then([](pplx::task<void> task)
					{
						// This continuation transforms a potentially cancelled task to a completed task
						try
						{
							task.get();
						}
						catch (const pplx::task_canceled ex)
						{
							// Do nothing
						}
						catch (...)
						{
							throw;
						}
					});
		}

		return _disconnectTask;
	}

	pplx::task<void> Scene_Impl::disconnectAllPeers(pplx::cancellation_token ct)
	{
		std::vector<pplx::task<void>> disconnectingPeers;

		{
			std::lock_guard<std::recursive_mutex> lg(_mutex);

			if (_connectedPeers.size() == 0)
			{
				return pplx::task_from_result(pplx::task_options(ct));
			}

			auto connectedPeers = _connectedPeers;
			for (auto pair : connectedPeers)
			{
				auto peer = pair.second;
				disconnectingPeers.push_back(peer->disconnect(ct));
			}
		}

		auto logger = _logger;
		auto wScene = STORM_WEAK_FROM_THIS();

		return pplx::when_all(disconnectingPeers.begin(), disconnectingPeers.end())
			.then([ct, logger, disconnectingPeers, wScene](pplx::task<void> task)
				{
					try
					{
						task.get();
					}
					catch (const std::exception&)
					{
						// Errors will be displayed later
						if (auto scene = wScene.lock())
						{
							std::lock_guard<std::recursive_mutex> lg(scene->_mutex);
							scene->_connectedPeers.clear();
						}
					}

					for (auto disconnectingPeerTask : disconnectingPeers)
					{
						disconnectingPeerTask.then([logger](pplx::task<void> continuation)
							{
								try
								{
									continuation.get();
								}
								catch (const std::exception& ex)
								{
									// Do nothing but display errors
									logger->log(LogLevel::Error, "Scene", "Peer disconnection failed", ex.what());
								}
							});
					}
				});
	}

	pplx::task<void> Scene_Impl::disconnectFromHost(pplx::cancellation_token ct)
	{
		if (_connectionState == ConnectionState::Connected)
		{
			auto client = _client.lock();
			if (client)
			{
				return client->disconnect(this->shared_from_this(), false, "", ct);
			}
			else
			{
				return pplx::task_from_result(pplx::task_options(ct));
			}
		}
		else if (_connectionState == ConnectionState::Connecting)
		{
			// We try to cancel the connection
			_connectionCts.cancel();
			// If that's too late, we disconnect the scene when the connection is finished
			auto wScene = STORM_WEAK_FROM_THIS();
			std::lock_guard<std::recursive_mutex> lg(_mutex);
			return _connectTask.then([wScene, ct](pplx::task<void> task)
				{
					try
					{
						task.get();
					}
					catch (const std::exception&)
					{
						// If the connection failed, we stop here
						return pplx::task_from_result(pplx::task_options(ct));
					}

					// If the connection finished successfully, we disconnect the scene
					if (auto scene = wScene.lock())
					{
						return scene->disconnect(ct);
					}

					return pplx::task_from_result(pplx::task_options(ct));
				});
		}
		else
		{
			return pplx::task_from_result(pplx::task_options(ct));
		}
	}

	const DependencyScope& Scene_Impl::dependencyResolver() const
	{
		return _scope;
	}

	void Scene_Impl::completeConnectionInitialization(ConnectionResult& cr)
	{
		_host = std::make_shared<ScenePeer>(_peer, cr.SceneHandle, _remoteRoutes, this->shared_from_this());
		for (auto pair : _localRoutes)
		{
			pair.second->setHandle(cr.RouteMappings[pair.first]);
			_handlers[pair.second->handle()] = pair.second->handlers;
		}
	}

	void Scene_Impl::handleMessage(Packet_ptr packet)
	{
		_onPacketReceived(packet);

		uint16 routeHandle;
		packet->stream >> routeHandle;

		if (mapContains(_handlers, routeHandle))
		{
			Route_ptr route;
			for (auto routeIt = _localRoutes.begin(); routeIt != _localRoutes.end(); ++routeIt)
			{
				if (routeIt->second->handle() == routeHandle)
				{
					route = routeIt->second;
					break;
				}
			}

			if (packet->connection->sessionId() == _host->id() && !((int)route->filter() & (int)Stormancer::MessageOriginFilter::Host))
			{
				return; // The route doesn't accept messages from the scene host.
			}

			if (packet->connection->sessionId() != _host->id() && !((int)route->filter() & (int)Stormancer::MessageOriginFilter::Peer))
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

	std::vector<std::shared_ptr<IScenePeer>> Scene_Impl::remotePeers() const
	{
		std::vector<std::shared_ptr<IScenePeer>> container;
		container.push_back(host());
		return container;
	}

	std::shared_ptr<IScenePeer> Scene_Impl::host() const
	{
		return _host;
	}

	bool Scene_Impl::isHost() const
	{
		return _isHost;
	}

	const std::unordered_map<std::string, std::shared_ptr<IP2PScenePeer>>& Scene_Impl::connectedPeers() const
	{
		return _connectedPeers;
	}

	Event<std::shared_ptr<IP2PScenePeer>> Scene_Impl::onPeerConnected() const
	{
		return _onPeerConnected;
	}

	Event<std::shared_ptr<IP2PScenePeer>> Scene_Impl::onPeerDisconnected() const
	{
		return _onPeerDisconnected;
	}

	rxcpp::observable<P2PConnectionStateChangedArgs> Scene_Impl::p2pConnectionStateChanged() const
	{
		return _p2pConnectionStateChangedObservable.get_observable();
	}

	std::shared_ptr<P2PTunnel> Scene_Impl::registerP2PServer(const std::string& p2pServerId)
	{
		auto p2p = dependencyResolver().resolve<P2PService>();
		return p2p->registerP2PServer(this->id() + "." + p2pServerId);
	}

	pplx::task<std::shared_ptr<IP2PScenePeer>> Scene_Impl::openP2PConnection(const std::string& p2pToken, pplx::cancellation_token ct)
	{
		pplx::task_options options(_dispatcher);
		options.set_cancellation_token(ct);

		auto p2pService = dependencyResolver().resolve<P2PService>();
		auto wScene = STORM_WEAK_FROM_THIS();
		auto routes = _host->routes();

		return p2pService->openP2PConnection(_host->connection(), p2pToken, ct)
			.then([wScene, p2pService, routes, ct](std::shared_ptr<IConnection> p2pConnection)
				{
					auto scene = LockOrThrow(wScene);

					auto& handles = *p2pConnection->dependencyResolver().resolve<std::vector<std::weak_ptr<Scene_Impl>>>();
					uint8 handle = 0;
					bool success = false;

					for (uint8 i = 0; i < 150; i++)
					{
						if (!handles[i].lock())
						{
							handle = i + (uint8)MessageIDTypes::ID_SCENES;
							handles[i] = scene;
							success = true;
							break;
						}
					}
					if (!success)
					{
						return pplx::task_from_exception<std::shared_ptr<IP2PScenePeer>>(std::runtime_error("Failed to generate handle for scene."));
					}

					P2PConnectToSceneMessage connectToSceneMessage;
					connectToSceneMessage.sceneId = scene->address().toUri();
					connectToSceneMessage.sceneHandle = handle;
					for (auto r : scene->localRoutes())
					{
						if ((byte)r->filter() & (byte)MessageOriginFilter::Peer)
						{
							RouteDto routeDto;
							routeDto.Handle = r->handle();
							routeDto.Name = r->name();
							routeDto.Metadata = r->metadata();
							connectToSceneMessage.routes << routeDto;
						}
					}
					connectToSceneMessage.connectionMetadata = p2pConnection->metadata();
					connectToSceneMessage.sceneMetadata = scene->getSceneMetadata();

					auto result = scene->sendSystemRequest<P2PConnectToSceneMessage, P2PConnectToSceneMessage>(p2pConnection, (byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, connectToSceneMessage, ct)
						.then([wScene, p2pConnection, p2pService](P2PConnectToSceneMessage connectToSceneMessage)
							{
								auto scene = LockOrThrow(wScene);
								auto scenePeer = scene->peerConnected(p2pConnection, p2pService, connectToSceneMessage);
								return scenePeer;
							}, ct);

					result.then([wScene, p2pConnection, ct](std::shared_ptr<IP2PScenePeer> peer)
						{
							if (auto scene = wScene.lock())
							{
								scene->sendSystemRequest<void>(p2pConnection, (byte)SystemRequestIDTypes::ID_CONNECTED_TO_SCENE, scene->address().toUri(), ct)
									.then([wScene, peer](pplx::task<void> t)
										{
											try
											{
												t.get();
											}
											catch (const std::exception&)
											{
											}

											if (auto scene = wScene.lock())
											{
												scene->raisePeerConnected(peer->sessionId());
											}
										});
							}
						});

					return result;
				}, options);
	}

	const std::unordered_map<std::string, std::string>& Scene_Impl::getSceneMetadata()
	{
		return _metadata;
	}

	std::shared_ptr<IP2PScenePeer> Scene_Impl::peerConnected(std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> p2pService, P2PConnectToSceneMessage connectToSceneMessage)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);
		auto it = _connectedPeers.find(connection->sessionId());

		if (it == _connectedPeers.end())
		{
			auto wScene = STORM_WEAK_FROM_THIS();
			auto scenePeer = std::make_shared<P2PScenePeer>(wScene, connection, p2pService, connectToSceneMessage);

			_connectedPeers.emplace(connection->sessionId(), scenePeer);

			return scenePeer;
		}
		else
		{
			return it->second;
		}
	}

	std::weak_ptr<IConnection> Scene_Impl::hostConnection()
	{
		return _peer;
	}

	void Scene_Impl::raisePeerConnected(const std::string& sessionId)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);
		auto it = _connectedPeers.find(sessionId);
		if (it != _connectedPeers.end())
		{
			auto wScene = STORM_WEAK_FROM_THIS();
			_dispatcher->post([wScene, it]()
				{
					if (auto scene = wScene.lock())
					{
						scene->_onPeerConnected(it->second);
					}
				});
		}
	}

	void Scene_Impl::raisePeerDisconnected(const std::string& sessionId)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);
		auto it = _connectedPeers.find(sessionId);
		if (it != _connectedPeers.end())
		{
			auto peer = it->second;
			_connectedPeers.erase(it);
			auto wScene = STORM_WEAK_FROM_THIS();
			_dispatcher->post([wScene, peer]()
				{
					auto scene = wScene.lock();
					if (scene)
					{
						scene->_onPeerDisconnected(peer);
					}
				}
			);
		}
	}

	Event<Packet_ptr> Scene_Impl::onPacketReceived()
	{
		return _onPacketReceived;
	}

	pplx::task<void> Scene_Impl::setConnectionState(ConnectionState state, bool async)
	{
		if (async)
		{
			auto scene = this->shared_from_this();
			// The connection state change and the event that signal it must be triggered synchronously on the game's dispatcher.
			return pplx::create_task([scene, state]//capture this because we don't want the object to be destroyed BEFORE we run this method.
			{
				scene->setConnectionStateInternal(state);
			}, _dispatcher);
		}
		else
		{
			setConnectionStateInternal(state);
			return pplx::task_from_result();
		}
	}

	pplx::task<std::shared_ptr<P2PScenePeer>> Scene_Impl::createScenePeerFromP2PConnection(std::shared_ptr<IConnection> /*connection*/, pplx::cancellation_token /*ct*/)
	{
		throw std::runtime_error("Not implemented");
	}

	void Scene_Impl::setConnectionStateInternal(ConnectionState state)
	{
		std::lock_guard<std::mutex> lg(_connectionStateMutex);

		if (_connectionState != state && !_connectionStateObservableCompleted)
		{
			_connectionState = state;
			// on_next and on_completed handlers might delete this. Do not use this at all after calling the handlers.
			_connectionStateObservableCompleted = (state == ConnectionState::Disconnected);
			auto subscriber = _sceneConnectionStateObservable.get_subscriber();

			auto scene = this->shared_from_this();
			switch (state.state)
			{
			case ConnectionState::Connecting:
				for (auto plugin : _plugins)
				{
					plugin->sceneConnecting(scene);
				}
				break;
			case ConnectionState::Connected:
				for (auto plugin : _plugins)
				{
					plugin->sceneConnected(scene);
				}
				break;
			case ConnectionState::Disconnecting:
				for (auto plugin : _plugins)
				{
					plugin->sceneDisconnecting(scene);
				}
				break;
			case ConnectionState::Disconnected:
				for (auto plugin : _plugins)
				{
					plugin->sceneDisconnected(scene);
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
}
