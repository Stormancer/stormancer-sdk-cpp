#pragma once
#include "Users/Users.hpp"
#include "stormancer/IPlugin.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/ITokenHandler.h"
#include "stormancer/Utilities/TaskUtilities.h"

namespace Stormancer
{
	namespace GameSessions
	{
		enum class P2PRole
		{
			Host,
			Client
		};

		enum class PlayerStatus
		{
			NotConnected = 0,
			Connected = 1,
			Ready = 2,
			Faulted = 3,
			Disconnected = 4
		};

		struct SessionPlayer
		{
		public:
			SessionPlayer(std::string playerId, PlayerStatus status, bool isHost = false)
				: playerId(playerId)
				, status(status)
				, isHost(isHost)
			{
			}

			std::string playerId;
			PlayerStatus status;
			bool isHost;
		};

		struct ServerStartedMessage
		{
		public:
			std::string p2pToken;
			MSGPACK_DEFINE(p2pToken);
		};

		struct PlayerUpdate
		{
		public:
			std::string userId;
			int status;
			std::string data;
			bool isHost;

			MSGPACK_DEFINE(userId, status, data, isHost);
		};



		struct GameSessionConnectionParameters
		{
			bool isHost;
			std::string hostMap;
			std::string endpoint;
		};

		class GameSessionsPlugin;

		/// <summary>
		/// public gamesession API
		/// </summary>
		/// <example>
		/// std::shared_ptr&lt;GameSession&gt; gs = client->dependencyResolver().resolve<GameSession>();
		/// </example>
		class GameSession
		{
		public:
			virtual ~GameSession() = default;

			virtual pplx::task<GameSessionConnectionParameters> connectToGameSession(std::string token, std::string mapName = "", bool openTunnel = true, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
			virtual pplx::task<void> setPlayerReady(const std::string& data = "", pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

			template<typename TServerResult, typename TClientResult>
			pplx::task<TServerResult> postResult(const TClientResult& clientResult, pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				StreamWriter streamWriter = [clientResult](obytestream& stream)
				{
					Serializer serializer;
					serializer.serialize(stream, clientResult);
				};
				return postResult(streamWriter, ct)
					.then([](Packetisp_ptr packet)
						{
							Serializer serializer;
							TServerResult serverResult = serializer.deserializeOne<TServerResult>(packet->stream);
							return serverResult;
						});
			}

			virtual pplx::task<Packetisp_ptr> postResult(const StreamWriter& streamWriter, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

			virtual pplx::task<std::string> getUserFromBearerToken(const std::string& token) = 0;
			virtual pplx::task<void> disconnectFromGameSession() = 0;

			Event<void> onAllPlayersReady;

			Event<GameSessionConnectionParameters> onRoleReceived;
			Event<GameSessionConnectionParameters> onTunnelOpened;

			Event<SessionPlayer, std::string> onPlayerStateChanged;

			/// <summary>
			/// Gets the underlying scene of the current gamesession, an empty shared ptr otherwise
			/// </summary>
			virtual std::shared_ptr<Scene> scene() = 0;

			/// <summary>
			/// Event fired when the client connects to a gamesession scene
			/// </summary>
			Event<std::shared_ptr<Scene>> onConnectingToScene;
			/// <summary>
			/// Event fired when the client disconnects from a gamesession scene
			/// </summary>
			Event<std::shared_ptr<Scene>> onDisconnectingFromScene;

			/// <summary>
			/// Get the P2P Host peer for this Game Session.
			/// </summary>
			/// <remarks>
			/// Players in a game session are connected together through a star topology centered on a single player, the Host.
			/// The players who are not the Host are called Clients.
			/// </remarks>
			/// <returns>
			/// The <c>IP2PScenePeer</c> for the host of the session.
			/// If you are the host, or you are not yet connected to a game session, it will be nullptr.
			/// </returns>
			virtual std::shared_ptr<IP2PScenePeer> getSessionHost() const = 0;

			/// <summary>
			/// Check whether you are the P2P Host of the Game Session.
			/// </summary>
			/// <seealso cref="getSessionHost()"/>
			/// <returns>
			/// <c>true</c> if you are the host of the session ; <c>false</c> if you are a client, or if you are not connected to a game session.
			/// </returns>
			virtual bool isSessionHost() const = 0;

			/// <summary>
			/// Event that is triggered when a host migration happens.
			/// </summary>
			/// <remarks>
			/// Host migration is not currently supported.
			/// </remarks>
			Event<std::shared_ptr<IP2PScenePeer>> onSessionHostChanged;
		};


		namespace details
		{
			constexpr char GAMESESSION_P2P_SERVER_ID[] = "GameSession";

			class GameSessionService :public std::enable_shared_from_this<GameSessionService>
			{
				friend class ::Stormancer::GameSessions::GameSessionsPlugin;
			public:
				GameSessionService(std::weak_ptr<Scene> scene) :
					_scene(scene),
					_logger(scene.lock()->dependencyResolver().resolve<ILogger>())
				{
				}


				pplx::task<std::shared_ptr<Stormancer::IP2PScenePeer>> initializeP2P(std::string p2pToken, bool openTunnel, pplx::cancellation_token ct)
				{
					ct = linkTokenToDisconnection(ct);
					auto scene = _scene.lock();
					if (!scene)
					{
						_logger->log(LogLevel::Error, "gamession.p2ptoken", "scene deleted");
						return pplx::task_from_exception<std::shared_ptr<Stormancer::IP2PScenePeer>>(std::runtime_error("scene deleted"), ct);
					}

					_logger->log(LogLevel::Trace, "gamession.p2ptoken", "recieved p2p token");
					if (_receivedP2PToken)
					{
						return pplx::task_from_result<std::shared_ptr<Stormancer::IP2PScenePeer>>(nullptr, pplx::task_options(ct));
					}

					_receivedP2PToken = true;
					_waitServerTce.set();
					if (p2pToken.empty()) // Host
					{
						_logger->log(LogLevel::Trace, "gamession.p2ptoken", "received empty p2p token: I'm the host.");
						_myP2PRole = P2PRole::Host;
						onRoleReceived(P2PRole::Host);
						_waitServerTce.set();
						if (openTunnel)
						{
							_tunnel = scene->registerP2PServer(GAMESESSION_P2P_SERVER_ID);
						}
						return pplx::task_from_result<std::shared_ptr<IP2PScenePeer>>(nullptr, pplx::task_options(ct));
					}
					else // Client
					{
						_logger->log(LogLevel::Trace, "gamession.p2ptoken", "received valid p2p token: I'm a client.");

						std::weak_ptr<GameSessionService> wThat = this->shared_from_this();
						return scene->openP2PConnection(p2pToken, ct)
							.then([wThat, ct, openTunnel](std::shared_ptr<IP2PScenePeer> p2pPeer)
								{
									auto that = wThat.lock();
									if (that)
									{
										that->_myP2PRole = P2PRole::Client;
										that->onRoleReceived(P2PRole::Client);
										if (that->_onConnectionOpened)
										{
											that->_onConnectionOpened(p2pPeer);
										}

										if (openTunnel)
										{
											return p2pPeer->openP2PTunnel(GAMESESSION_P2P_SERVER_ID, ct)
												.then([wThat, p2pPeer](std::shared_ptr<P2PTunnel> guestTunnel)
													{
														auto that = wThat.lock();
														if (that)
														{
															that->_tunnel = guestTunnel;
															that->onTunnelOpened(guestTunnel);
														}
														return p2pPeer;
													}, ct);
										}
										else
										{
											return pplx::task_from_result(p2pPeer, ct);
										}
									}
									else
									{
										return pplx::task_from_exception<std::shared_ptr<IP2PScenePeer>>(std::runtime_error("Service destroyed"), ct);
									}
								}, ct)
							.then([wThat](pplx::task<std::shared_ptr<IP2PScenePeer>> t)
								{
									auto that = wThat.lock();
									try
									{
										auto p = t.get();
										return p;
									}
									catch (const std::exception& ex)
									{
										if (that)
										{
											that->_onConnectionFailure(ex.what());
											that->_logger->log(ex);
										}
										throw;
									}
								}, ct);
					}
				}

				pplx::task<void> waitServerReady(pplx::cancellation_token token)
				{
					token = linkTokenToDisconnection(token);
					return pplx::create_task(_waitServerTce, pplx::task_options(token));
				}

				std::vector<SessionPlayer> getConnectedPlayers()
				{
					return this->_users;
				}

				std::weak_ptr<Scene> getScene()
				{
					return _scene;
				}

				pplx::task<std::string> getUserFromBearerToken(std::string token)
				{
					if (auto scene = _scene.lock())
					{
						auto rpc = scene->dependencyResolver().resolve<RpcService>();
						return rpc->rpc<std::string, std::string>("GameSession.GetUserFromBearerToken", token);
					}
					else
					{
						throw pplx::task_from_exception<std::string>(std::runtime_error("Scene destroyed"));
					}
				}

				pplx::task<std::string> requestP2PToken(pplx::cancellation_token ct)
				{
					if (auto scene = _scene.lock())
					{
						ct = linkTokenToDisconnection(ct);
						auto rpc = scene->dependencyResolver().resolve<RpcService>();
						return rpc->rpc<std::string, int>("GameSession.GetP2PToken", ct, 1);
					}
					else
					{
						throw pplx::task_from_exception<std::string>(std::runtime_error("Scene destroyed"), ct);
					}
				}

				pplx::task<void> reset(pplx::cancellation_token ct)
				{
					ct = linkTokenToDisconnection(ct);
					auto scene = _scene.lock();
					if (!scene)
					{
						return pplx::task_from_exception<void>(std::runtime_error("Scene deleted"), ct);
					}

					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc("gamesession.reset", ct);
				}

				pplx::task<void> disconnect()
				{
					auto scene = _scene.lock();
					if (!scene)
					{
						return pplx::task_from_result();
					}

					return scene->disconnect();
				}

				void onDisconnecting()
				{
					_tunnel = nullptr;
					_users.clear();
					_disconnectionCts.cancel();
				}

				void ready(std::string data)
				{
					auto scene = _scene.lock();
					if (!scene)
					{
						_logger->log(LogLevel::Error, "category", "scene deleted");
						return;
					}

					scene->send("player.ready", [data](obytestream& stream)
						{
							msgpack::pack(stream, data);
						});
				}

				pplx::task<Packetisp_ptr> sendGameResults(const StreamWriter& streamWriter, pplx::cancellation_token ct)
				{
					auto scene = _scene.lock();
					if (!scene)
					{
						return pplx::task_from_exception<Packetisp_ptr>(std::runtime_error("Scene deleted"));
					}

					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<Packetisp_ptr>("gamesession.postresults", ct, streamWriter);
				}

				P2PRole getMyP2PRole() const { return _myP2PRole; }

				Event<void> onAllPlayersReady;
				Event<P2PRole> onRoleReceived;
				Event<std::shared_ptr<Stormancer::P2PTunnel>> onTunnelOpened;
				Event<void> onShutdownReceived;
				Event<SessionPlayer, std::string> onPlayerStateChanged;
			private:

				void initialize()
				{
					_disconnectionCts = pplx::cancellation_token_source();
					std::weak_ptr<GameSessionService> wThat = this->shared_from_this();



					_scene.lock()->addRoute("player.update", [wThat](Packetisp_ptr packet)
						{
							auto that = wThat.lock();
							if (that)
							{
								auto update = packet->readObject<Stormancer::GameSessions::PlayerUpdate>();
								SessionPlayer player(update.userId, (PlayerStatus)update.status, update.isHost);

								auto end = that->_users.end();
								auto it = std::find_if(that->_users.begin(), end, [player](SessionPlayer p) { return p.playerId == player.playerId; });
								if (it == end)
								{
									that->_users.push_back(player);
								}
								else
								{
									*it = player;
								}
								that->onPlayerStateChanged(player, update.data);
							}
						});

					_scene.lock()->addRoute("players.allReady", [wThat](Packetisp_ptr packet) {
						auto that = wThat.lock();
						if (that)
						{
							that->onAllPlayersReady();
						}
						});
				}

				pplx::cancellation_token linkTokenToDisconnection(pplx::cancellation_token tokenToLink)
				{
					if (tokenToLink.is_cancelable())
					{
						auto tokens = { tokenToLink, _disconnectionCts.get_token() };
						return pplx::cancellation_token_source::create_linked_source(tokens.begin(), tokens.end()).get_token();
					}
					else
					{
						return _disconnectionCts.get_token();
					}
				}

			private:



				std::shared_ptr<P2PTunnel> _tunnel;
				Event<std::string> _onConnectionFailure;
				std::function<void(std::shared_ptr<Stormancer::IP2PScenePeer>)> _onConnectionOpened;
				pplx::task_completion_event<void> _waitServerTce;

				std::weak_ptr<Scene> _scene;
				std::vector<SessionPlayer> _users;
				std::shared_ptr<Stormancer::ILogger> _logger;
				bool _receivedP2PToken = false;
				pplx::cancellation_token_source _disconnectionCts;
				P2PRole _myP2PRole = P2PRole::Client;

			};


			//Private gamesession container implementation that deals with the lifecycle of a gamesession connection
			struct GameSessionContainer
			{
				GameSessionContainer() {};

				//Keep game finder scene alive
				pplx::task<std::shared_ptr<Scene>> scene;
				std::string sceneId;
				std::string mapName;

				pplx::cancellation_token cancellationToken()
				{
					return cts.get_token();
				}

				pplx::task<std::shared_ptr<GameSessionService>> service()
				{
					return scene.then([](std::shared_ptr<Scene> scene) {return scene->dependencyResolver().resolve<GameSessionService>(); });
				}

				pplx::task_completion_event<void> _hostIsReadyTce;
				pplx::task_completion_event<GameSessionConnectionParameters> sessionReadyTce;

				pplx::task<GameSessionConnectionParameters> sessionReadyAsync() {
					return pplx::create_task(sessionReadyTce);
				}

				std::shared_ptr<IP2PScenePeer>	p2pHost;

				Subscription allPlayerReady;
				Subscription onRoleReceived;
				Subscription onTunnelOpened;
				Subscription onShutdownRecieved;
				Subscription onPlayerChanged;




				~GameSessionContainer()
				{
					cts.cancel();
					_hostIsReadyTce.set_exception(pplx::task_canceled());
					try
					{
						pplx::create_task(_hostIsReadyTce).get();
					}
					catch (...) {}

				}
			private:
				pplx::cancellation_token_source cts;
			};

			class GameSession_Impl : public GameSession, public std::enable_shared_from_this<GameSession_Impl>
			{
				friend class ::Stormancer::GameSessions::GameSessionsPlugin;
			public:
				GameSession_Impl(std::weak_ptr<IClient> client, std::shared_ptr<ITokenHandler> tokens, std::shared_ptr<ILogger> logger, std::shared_ptr<IActionDispatcher> dispatcher)
					: _logger(logger)
					, _tokens(tokens)
					, _wClient(client)
					, _currentGameSession(nullptr)
					, _dispatcher(dispatcher)
				{
				}

				pplx::task<GameSessionConnectionParameters> connectToGameSession(std::string token, std::string mapName, bool openTunnel, pplx::cancellation_token ct)
				{
					std::lock_guard<std::mutex> lg(_lock);



					if (token.empty())
					{
						return pplx::task_from_exception<GameSessionConnectionParameters>(std::runtime_error("Empty connection token"));
					}

					// Client should never be null here
					auto dispatcher = _wClient.lock()->dependencyResolver().resolve<IActionDispatcher>();

					std::weak_ptr<GameSession_Impl> wThat = this->shared_from_this();

					_currentGameSession = std::make_shared<GameSessionContainer>();
					_currentGameSession->mapName = mapName;
					if (ct.is_cancelable())
					{
						ct.register_callback([wThat]() {
							if (auto that = wThat.lock())
							{
								that->_currentGameSession = nullptr;
							}
							});
					}

					auto infos = _tokens->decodeToken(token);
					_currentGameSession->sceneId = infos.tokenData.SceneId;

					auto cancellationToken = _currentGameSession->cancellationToken();
					std::weak_ptr<GameSessionContainer> wContainer = _currentGameSession;

					auto scene = connectToGameSessionImpl(token, openTunnel, cancellationToken, wContainer)
						.then([wThat, openTunnel, cancellationToken, wContainer](std::shared_ptr<Scene> scene)
							{
								auto that = wThat.lock();

								if (!that)
								{
									throw std::runtime_error("Game session deleted");
								}

								that->_logger->log(LogLevel::Trace, "GameSession", "Requesting P2P token", "");
								return that->requestP2PToken(scene, cancellationToken)
									.then([scene, openTunnel, cancellationToken, wThat](pplx::task<std::string> task)
										{
											auto that = wThat.lock();

											auto service = scene->dependencyResolver().resolve<GameSessionService>();
											auto logger = scene->dependencyResolver().resolve<ILogger>();
											try
											{
												auto token = task.get();
												logger->log(LogLevel::Trace, "GameSession", "Initialize P2Ps", "");
												return service->initializeP2P(token, openTunnel, cancellationToken);
											}
											catch (std::exception& e)
											{
												e.what();
												throw std::runtime_error("Cannot get p2pToken");
											}
										}, cancellationToken)
									.then([scene, wContainer](std::shared_ptr<IP2PScenePeer> peer)
										{
											auto c = wContainer.lock();
											if (!c)
											{
												throw pplx::task_canceled("Connecting to game session cancelled");
											}
											c->p2pHost = peer;
											return scene;
										}, cancellationToken);

							}, cancellationToken);

					_currentGameSession->scene = scene;

					return scene
						.then([wThat, cancellationToken, wContainer](std::shared_ptr<Scene> scene)
							{
								auto c = wContainer.lock();
								if (!c)
								{
									throw pplx::task_canceled();
								}
								if (auto that = wThat.lock())
								{
									that->_logger->log(LogLevel::Trace, "GameSession", "Waiting role", "");
								}

								auto hostReadyTce = c->_hostIsReadyTce;
								return c->sessionReadyAsync().then([wThat, hostReadyTce, cancellationToken](GameSessionConnectionParameters gameSessionConnectionParameters)
									{
										if (auto that = wThat.lock())
										{
											if (gameSessionConnectionParameters.isHost) // Host = connect immediately
											{
												return pplx::task_from_result(gameSessionConnectionParameters, cancellationToken);
											}
											else // Client = waiting for host to be ready
											{

												that->_logger->log(LogLevel::Trace, "GameSession", "Waiting host is ready", "");

												return pplx::create_task(hostReadyTce, cancellationToken)
													.then([wThat, gameSessionConnectionParameters]()
														{
															if (auto that = wThat.lock())
															{
																that->_logger->log(LogLevel::Trace, "GameSession", "Host is ready", "");
															}
															return gameSessionConnectionParameters;
														}, cancellationToken);
											}
										}
										else
										{
											throw std::runtime_error("Game session deleted");
										}
									});
							}, ct)
						.then([wThat](pplx::task<GameSessionConnectionParameters> task)
							{
								try
								{
									task.get();
								}
								catch (...)
								{
									if (auto that = wThat.lock())
									{
										std::exception_ptr ptrEx = std::current_exception();
										return that->disconnectFromGameSession().then([wThat, ptrEx](pplx::task<void> task) -> GameSessionConnectionParameters
											{
												try
												{
													task.get();
												}
												catch (const std::exception& ex)
												{
													if (auto that = wThat.lock())
													{
														auto logger = that->_wClient.lock()->dependencyResolver().resolve<ILogger>();
														logger->log(LogLevel::Warn, "GameSessionConnection", "Cannot disconnect from game session after connection timeout or cancel.", ex.what());
													}
												}

												std::rethrow_exception(ptrEx);
											});
									}
								}
								return task;
							}, dispatcher);
				}


				pplx::task<void> setPlayerReady(const std::string& data, pplx::cancellation_token ct)
				{
					if (auto dispatcher = _dispatcher.lock())
					{
						return getCurrentGameSession(ct).then([data](std::shared_ptr<Scene> scene)
						{
							if (scene)
							{
								auto gameSessionService = scene->dependencyResolver().resolve<GameSessionService>();
								gameSessionService->ready(data);
							}
							else
							{
								throw std::runtime_error("Not connected to any game session");
							}
						}, dispatcher);
					}
					else
					{
						STORM_RETURN_TASK_FROM_EXCEPTION(void, PointerDeletedException());
					}
				}

				pplx::task<Packetisp_ptr> postResult(const StreamWriter& streamWriter, pplx::cancellation_token ct)
				{
					return getCurrentGameSession(ct)
						.then([streamWriter, ct](std::shared_ptr<Scene> scene)
							{
								if (scene)
								{
									auto gameSessionService = scene->dependencyResolver().resolve<GameSessionService>();
									return gameSessionService->sendGameResults(streamWriter, ct);
								}
								else
								{
									throw std::runtime_error("Not connected to any game session");
								}
							});
				}

				pplx::task<std::string> getUserFromBearerToken(const std::string& token)
				{
					return getCurrentGameSession()
						.then([token](std::shared_ptr<Scene> scene)
							{
								if (scene)
								{
									auto gameSessionService = scene->dependencyResolver().resolve<GameSessionService>();
									return gameSessionService->getUserFromBearerToken(token);
								}
								else
								{
									throw std::runtime_error("Not connected to any game session");
								}
							});
				}

				pplx::task<void> disconnectFromGameSession()
				{
					std::weak_ptr<GameSession_Impl> wThat = this->shared_from_this();
					// catch err
					return this->getCurrentGameSession()
						.then([wThat](pplx::task<std::shared_ptr<Scene>> task)
							{
								try
								{
									auto scene = task.get();
									if (scene)
									{
										if (auto that = wThat.lock())
										{
											that->_logger->log(LogLevel::Trace, "GameSession", "Disconnecting from previous games session", scene->id());
										}
										auto gameSessionService = scene->dependencyResolver().resolve<GameSessionService>();

										return gameSessionService->disconnect();
									}
									else
									{
										return pplx::task_from_result();
									}
								}
								catch (std::exception&)
								{
									return pplx::task_from_result();
								}
							});
				}


				std::shared_ptr<Scene> scene()
				{

					if (this->_currentGameSession && this->_currentGameSession->scene.is_done())
					{
						return this->_currentGameSession->scene.get();
					}
					else
					{
						return nullptr;
					}
				}

				std::shared_ptr<IP2PScenePeer> getSessionHost() const
				{
					// Copy the task to avoid a possible race condition with it being reassigned while we are inside this method
					auto container = _currentGameSession;
					if (!container || !container->scene.is_done())
					{
						return nullptr;
					}
					return container->p2pHost;

				}

				bool isSessionHost() const
				{
					auto container = _currentGameSession;
					if (!container || !container->scene.is_done())
					{
						return false;
					}

					auto session = container->scene.get();
					if (session)
					{
						auto service = session->dependencyResolver().resolve<GameSessionService>();
						return service->getMyP2PRole() == P2PRole::Host;
					}
					return false;
				}

			private:
				//methods
				pplx::task<std::shared_ptr<Scene>> connectToGameSessionImpl(std::string token, bool useTunnel, pplx::cancellation_token ct, std::weak_ptr<GameSessionContainer> wContainer)
				{
					std::weak_ptr<GameSession_Impl> wThat = this->shared_from_this();
					return _wClient.lock()->connectToPrivateScene(token, [wContainer, useTunnel, wThat](std::shared_ptr<Scene> scene) {

						auto gameSessionContainer = wContainer.lock();
						if (!gameSessionContainer)
						{
							throw pplx::task_canceled();
						}
						

						auto service = scene->dependencyResolver().resolve<GameSessionService>();

						gameSessionContainer->onRoleReceived = service->onRoleReceived.subscribe([wThat, useTunnel, wContainer](P2PRole role)
							{
								auto gameSessionContainer = wContainer.lock();
								auto that = wThat.lock();
								if (that && gameSessionContainer)
								{
									if ((role == P2PRole::Host) || (role == P2PRole::Client && !useTunnel))
									{
										GameSessionConnectionParameters gameSessionParameters;
										gameSessionParameters.endpoint = gameSessionContainer->mapName;
										gameSessionParameters.isHost = (role == P2PRole::Host);
										that->onRoleReceived(gameSessionParameters);
										gameSessionContainer->sessionReadyTce.set(gameSessionParameters);
									}
								}
							});
						if (useTunnel)
						{
							gameSessionContainer->onTunnelOpened = service->onTunnelOpened.subscribe([wThat, wContainer](std::shared_ptr<Stormancer::P2PTunnel> p2pTunnel)
								{
									auto gameSessionContainer = wContainer.lock();
									auto that = wThat.lock();
									if (gameSessionContainer && that)
									{
										GameSessionConnectionParameters gameSessionParameters;
										gameSessionParameters.isHost = false;
										gameSessionParameters.endpoint = p2pTunnel->ip + ":" + std::to_string(p2pTunnel->port);

										that->onTunnelOpened(gameSessionParameters);
										gameSessionContainer->sessionReadyTce.set(gameSessionParameters);
									}
								});
						}

						gameSessionContainer->allPlayerReady = service->onAllPlayersReady.subscribe([wThat]()
							{
								if (auto that = wThat.lock())
								{
									that->onAllPlayersReady();
								}
							});


						auto tce = gameSessionContainer->_hostIsReadyTce;
						gameSessionContainer->onPlayerChanged = service->onPlayerStateChanged.subscribe([wThat, tce](SessionPlayer player, std::string data)
							{
								if (auto that = wThat.lock())
								{
									that->onPlayerStateChanged(player, data);
									if (player.isHost && player.status == PlayerStatus::Ready)
									{
										tce.set();
									}
								}
							});

						}, ct);

				}
				pplx::task<std::shared_ptr<Scene>> getCurrentGameSession(pplx::cancellation_token ct = pplx::cancellation_token::none())
				{
					if (_currentGameSession)
					{
						return _currentGameSession->scene;
					}
					else
					{
						return pplx::task_from_result<std::shared_ptr<Scene>>(nullptr);
					}
				}
				pplx::task<std::string> requestP2PToken(std::shared_ptr<Scene> scene, pplx::cancellation_token ct)
				{
					std::weak_ptr<GameSession> wThat = this->shared_from_this();

					std::shared_ptr<GameSessionService> gameSessionService = scene->dependencyResolver().resolve<GameSessionService>();
					return gameSessionService->requestP2PToken(ct);
				}

				void onDisconnectingFromGameSession(std::shared_ptr<Scene> scene)
				{
					_currentGameSession = nullptr;
					onDisconnectingFromScene(scene);


				}

				//Fields
				std::shared_ptr<ILogger> _logger;
				std::shared_ptr<ITokenHandler> _tokens;
				std::weak_ptr<IActionDispatcher> _dispatcher;
				std::weak_ptr<IClient> _wClient;
				std::shared_ptr<GameSessionContainer> _currentGameSession;
				std::mutex _lock;
			};


		}



		class GameSessionsPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{

				auto name = scene->getHostMetadata("stormancer.gamesession");
				if (name.length() > 0)
				{
					builder.registerDependency<details::GameSessionService, Scene>().singleInstance();
				}

			}


			void sceneCreated(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.gamesession");
				if (name.length() > 0)
				{
					scene->dependencyResolver().resolve<details::GameSessionService>()->initialize();
				}
			}

			void sceneConnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.gamesession");
				if (name.length() > 0)
				{

					scene->dependencyResolver().resolve<GameSession>()->onConnectingToScene(scene);
				}
			}

			void sceneDisconnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.gamesession");
				if (name.length() > 0)
				{
					auto gameSession = scene->dependencyResolver().resolve<details::GameSessionService>();
					if (gameSession)
					{
						std::static_pointer_cast<details::GameSession_Impl>(scene->dependencyResolver().resolve<GameSession>())->onDisconnectingFromScene(scene);
						gameSession->onDisconnecting();
					}
				}
			}

			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency < details::GameSession_Impl, IClient, ITokenHandler, ILogger,IActionDispatcher >().as<GameSession>().singleInstance();
			}
		};
	}
}