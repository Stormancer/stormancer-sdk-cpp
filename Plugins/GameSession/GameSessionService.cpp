#include "stormancer/headers.h"
#include "stormancer/scene.h"
#include "stormancer/RPC/Service.h"
#include "GameSessionService.h"

namespace Stormancer
{

	struct GameSessionContainer
	{
		GameSessionContainer() {};

		//Keep game finder scene alive
		std::shared_ptr<Scene> scene;
		std::shared_ptr<GameSessionService> service()
		{
			return scene->dependencyResolver()->resolve<GameSessionService>();
		}

		Event<void>::Subscription AllPlayerReady;
		Event<std::string>::Subscription OnRoleRecieved;
		Event<std::shared_ptr<Stormancer::P2PTunnel>>::Subscription OnTunnelOpened;

		rxcpp::subscription SceneConnectionState;
		~GameSessionContainer()
		{
			auto logger = scene->dependencyResolver()->resolve<ILogger>();
			if (logger)
			{
				logger->log(LogLevel::Info, "GameSession", "Destroyed", scene->id());
			}
		}
	};

	GameSessionService::GameSessionService(std::weak_ptr<Scene> scene) :
		_onShutdownReceived([]() {}),
		_scene(scene),
		_logger(scene.lock()->dependencyResolver()->resolve<ILogger>())
	{
	}

	void GameSessionService::Initialize()
	{
		std::weak_ptr<GameSessionService> wThat = this->shared_from_this();

		_scene.lock()->addRoute("gameSession.shutdown", [wThat](Packetisp_ptr packet) {
			auto that = wThat.lock();
			if (that)
			{
				if (that->_onShutdownReceived)
				{
					that->_onShutdownReceived();
				}
			}
		});

		_scene.lock()->addRoute("player.update", [wThat](Packetisp_ptr packet) {
			auto that = wThat.lock();
			if (that)
			{
				auto update = packet->readObject<Stormancer::PlayerUpdate>();
				SessionPlayer player(update.UserId, (PlayerStatus)update.Status);

				auto end = that->_users.end();
				auto it = std::find_if(that->_users.begin(), end, [player](SessionPlayer p) { return p.PlayerId == player.PlayerId; });
				if (it == end)
				{
					that->_users.push_back(player);
				}
				else
				{
					*it = player;
				}
				that->_onConnectedPlayersChanged(SessionPlayerUpdateArg(player, update.Data));
			}
		});

		_scene.lock()->addRoute("players.allReady", [wThat](Packetisp_ptr packet) {
			auto that = wThat.lock();
			if (that)
			{
				that->OnAllPlayerReady();
			}
		});

		_scene.lock()->addRoute("player.p2ptoken", [wThat](Packetisp_ptr packet) {
			auto that = wThat.lock();
			if (that)
			{
				that->_waitServerTce.set();
				auto p2pToken = packet->readObject<std::string>();
				that->InitializeTunnel(p2pToken).then([wThat](pplx::task<void> task)
				{
					try {
						task.get();
					}
					catch (std::exception& ex)
					{
						if (auto that = wThat.lock())
						{
							that->_logger->log(LogLevel::Error, "GameSession", "An error occured during tunnel initialization", ex.what());
						}
					}
				});
			}
		});
	}

	pplx::task<void> GameSessionService::InitializeTunnel(std::string p2pToken)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			_logger->log(LogLevel::Error, "gamession.p2ptoken", "scene deleted");
			return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
		}

		_logger->log(LogLevel::Trace, "gamession.p2ptoken", "recieved p2p token");
		if (_receivedP2PToken)
		{
			return pplx::task_from_result();
		}

		_receivedP2PToken = true;
		if (p2pToken.empty()) //host
		{
			_logger->log(LogLevel::Trace, "gamession.p2ptoken", "received empty p2p token: I'm the host.");
			OnRoleReceived("HOST");
			_waitServerTce.set();
			_tunnel = scene->registerP2PServer(GAMESESSION_P2P_SERVER_ID);
			return pplx::task_from_result();
		}
		else //client
		{
			_logger->log(LogLevel::Trace, "gamession.p2ptoken", "received valid p2p token: I'm a client.");

			std::weak_ptr<GameSessionService> wThat = this->shared_from_this();
			return scene->openP2PConnection(p2pToken).then([wThat](std::shared_ptr<Stormancer::IP2PScenePeer> p2pPeer) {
				auto that = wThat.lock();
				if (that)
				{
					that->OnRoleReceived("CLIENT");
					if (that->_onConnectionOpened)
					{
						that->_onConnectionOpened(p2pPeer);
					}

					if (that->shouldEstablishTunnel)
					{
						return p2pPeer->openP2PTunnel(GAMESESSION_P2P_SERVER_ID).then([wThat](std::shared_ptr<Stormancer::P2PTunnel> guestTunnel)
						{
							auto that = wThat.lock();
							if (that)
							{
								that->_tunnel = guestTunnel;
								that->OnTunnelOpened(guestTunnel);
							}
						});
					}
					else
					{
						return pplx::task_from_result();
					}
				}
				else
				{
					return pplx::task_from_exception<void>(std::runtime_error("Service destroyed"));
				}
			}).then([wThat](pplx::task<void> t) {
				auto that = wThat.lock();
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					if (that)
					{
						that->_onConnectionFailure(ex.what());
						that->_logger->log(ex);
					}
				}
			});
		}
	}

	GameSessionService::~GameSessionService()
	{
		std::clog << "GameSessionService destroyed" << std::endl;
	}

	pplx::task<void> GameSessionService::waitServerReady(pplx::cancellation_token token)
	{
		return pplx::create_task(_waitServerTce, pplx::task_options(token));
	}

	std::vector<SessionPlayer> GameSessionService::GetConnectedPlayers()
	{
		return this->_users;
	}

	void GameSessionService::unsubscribeConnectedPlayersChanged(Action<SessionPlayerUpdateArg>::TIterator handle)
	{
		this->_onConnectedPlayersChanged.erase(handle);
	}

	std::function<void()> Stormancer::GameSessionService::OnConnectedPlayerChanged(std::function<void(SessionPlayer, std::string)> callback)
	{
		auto iterator = this->_onConnectedPlayersChanged.push_back([callback](SessionPlayerUpdateArg args) {callback(args.sessionPlayer, args.data); });
		return [iterator, this]() {
			unsubscribeConnectedPlayersChanged(iterator);
		};
	}

	void GameSessionService::OnP2PConnected(std::function<void(std::shared_ptr<Stormancer::IP2PScenePeer>)> callback)
	{
		_onConnectionOpened = callback;
	}

	void GameSessionService::OnShutdownReceived(std::function<void(void)> callback)
	{
		_onShutdownReceived = callback;
	}

	void GameSessionService::OnConnectionFailure(std::function<void(std::string)> callback)
	{
		_onConnectionFailure += callback;
	}

	std::weak_ptr<Scene> GameSessionService::GetScene()
	{
		return _scene;
	}

	pplx::task<std::string> GameSessionService::P2PTokenRequest()
	{
		if (auto scene = _scene.lock())
		{
			auto rpc = scene->dependencyResolver()->resolve<RpcService>();
			return rpc->rpc<std::string, int>("GameSession.GetP2PToken", 1);
		}
		else
		{
			throw pplx::task_from_exception<std::string>(std::runtime_error("Scene destroyed"));
		}
	}

	//pplx::task<void> GameSessionService::connect()
	//{
	//	auto scene = _scene.lock();
	//	if (!scene)
	//	{
	//		return pplx::task_from_exception<void>(std::runtime_error("Scene deleted"));
	//	}

	//	return scene->disconnect connect();
	//}

	pplx::task<void> GameSessionService::reset()
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene deleted"));
		}

		auto rpc = scene->dependencyResolver()->resolve<RpcService>();
		return rpc->rpcWriter("gamesession.reset", pplx::cancellation_token::none(), [](obytestream*) {});
	}

	pplx::task<void> GameSessionService::disconnect()
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_result();
		}

		return scene->disconnect();
	}

	void GameSessionService::__disconnecting()
	{
		_tunnel = nullptr;
		_users.clear();
	}

	void GameSessionService::ready(std::string data)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			_logger->log(LogLevel::Error, "category", "scene deleted");
			return;
		}

		scene->send("player.ready", [data](obytestream* stream) {
			msgpack::pack(stream, data);
		});
	}

	GameSession::GameSession(std::weak_ptr<IClient> client)
		: _wClient(client),
		_currentGameSession(pplx::task_from_result<std::shared_ptr<GameSessionContainer>>(nullptr))
	{
	}

	// Add cancelation token for timeout or cancel.
	pplx::task<GameSessionConnectionParameters> GameSession::ConnectToGameSession(std::string token, std::string mapName)
	{
		std::lock_guard<std::mutex> lg(_lock);
		if (!token.empty())
		{
			_gameSessionNegotiationTce = pplx::task_completion_event<GameSessionConnectionParameters>();

			_mapName = mapName;
			std::weak_ptr<GameSession> wThat = this->shared_from_this();
			auto connectionTask = DisconectFromGameSession().then([wThat, token]()
			{
				if (auto that = wThat.lock())
				{
					return that->connectToGameSessionImpl(token).then([wThat](pplx::task<std::shared_ptr<GameSessionContainer>> task)
					{
						if (auto that = wThat.lock()) {
							return task;
						}
						else
						{
							throw std::runtime_error("Game session destroyed");
						}
					});
				}
				else
				{
					throw std::runtime_error("Game session destroyed");
				}
			}).then([wThat](pplx::task<std::shared_ptr<GameSessionContainer>> task)
			{
				if (auto that = wThat.lock())
				{
					try
					{
						auto gameSessionContainer = task.get();
						return that->P2PTokenRequest(gameSessionContainer).then([gameSessionContainer](pplx::task<std::string> task)
						{
							auto service = gameSessionContainer->service();
							auto logger = gameSessionContainer->scene->dependencyResolver()->resolve<ILogger>();
							try
							{
								auto token = task.get();
								return service->InitializeTunnel(token);
							}
							catch (std::exception& e)
							{
								e.what();
								throw std::runtime_error("Cannot get p2pToken");
							}
						}).then([gameSessionContainer](pplx::task<void> t) {
							try
							{
								t.get();
								return gameSessionContainer;
							}
							catch (std::exception& e)
							{
								e.what();
								throw;
							}

						});
					}
					catch (std::exception&)
					{
						throw std::runtime_error("Game Session container doesn't exist");
					}
				}
				else
				{
					throw std::runtime_error("Game session deleted");
				}
			});

			auto completionTask = pplx::create_task(_gameSessionNegotiationTce, _wClient.lock()->dependencyResolver()->resolve<IActionDispatcher>());
			_currentGameSession = completionTask.then([connectionTask](GameSessionConnectionParameters) {
				return connectionTask;
			});

			return connectionTask.then([](std::shared_ptr<GameSessionContainer> c) {
				return c->service()->waitServerReady(pplx::cancellation_token::none());

			}).then([completionTask]() {
				return completionTask;
			});
		}
		else
		{
			return pplx::task_from_exception<GameSessionConnectionParameters>(std::runtime_error("Game session can't be connected without token"));
		}
	}


	pplx::task<void> GameSession::SetPlayerReady(std::string data)
	{
		return this->getCurrentGameSession().then([data](std::shared_ptr<GameSessionContainer> container) {
			if (container)
			{
				std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<Stormancer::GameSessionService>();
				gameSessionService->ready(data);
			}
			else
			{
				throw std::runtime_error("Not connected to any game session");
			}
		});
	}

	pplx::task<GameSessionResult> GameSession::PostResult(EndGameDto gameSessioResult)
	{
		return this->getCurrentGameSession().then([gameSessioResult](std::shared_ptr<GameSessionContainer> container) {
			if (container)
			{
				std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<Stormancer::GameSessionService>();
				return gameSessionService->sendGameResults<GameSessionResult, EndGameDto>(gameSessioResult);
			}
			else
			{
				throw std::runtime_error("Not connected to any game session");
			}
		});
	}

	pplx::task<void> GameSession::DisconectFromGameSession()
	{
		std::weak_ptr<GameSession> wThat = this->shared_from_this();
		// catch err
		return this->getCurrentGameSession().then([wThat](pplx::task<std::shared_ptr<GameSessionContainer>> task) {
			try
			{
				auto container = task.get();
				if (container)
				{
					std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<Stormancer::GameSessionService>();
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

	pplx::task<std::string> GameSession::P2PTokenRequest(std::shared_ptr<GameSessionContainer> container)
	{
		std::weak_ptr<GameSession> wThat = this->shared_from_this();
		if (container)
		{
			std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<Stormancer::GameSessionService>();
			return gameSessionService->P2PTokenRequest();
		}
		else
		{
			throw(std::runtime_error("GameSession doesn't exist"));
		}
	}

	pplx::task<std::shared_ptr<GameSessionContainer>> GameSession::getCurrentGameSession()
	{
		return _currentGameSession;
	}

	pplx::task<std::shared_ptr<GameSessionContainer>> GameSession::connectToGameSessionImpl(std::string token)
	{
		std::weak_ptr<GameSession> wThat = this->shared_from_this();
		return _wClient.lock()->connectToPrivateScene(token).then([wThat](std::shared_ptr<Scene> scene)
		{
			if (auto that = wThat.lock())
			{
				auto gameSessionContainer = std::make_shared<GameSessionContainer>();
				gameSessionContainer->scene = scene;

				gameSessionContainer->SceneConnectionState = scene->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state)
				{
					if (state == ConnectionState::Disconnected)
					{
						if (auto that = wThat.lock())
						{
							that->_currentGameSession = pplx::task_from_result<std::shared_ptr<GameSessionContainer>>(nullptr);
						}
					}
				});


				gameSessionContainer->OnRoleRecieved = gameSessionContainer->service()->OnRoleReceived.subscribe([wThat](std::string role)
				{
					if (auto that = wThat.lock())
					{
						if (role == "HOST")
						{
							GameSessionConnectionParameters gameSessionParameters;
							gameSessionParameters.isHost = role == "HOST" ? true : false;
							gameSessionParameters.Endpoint = that->_mapName;
							that->OnRoleRecieved(gameSessionParameters);
							that->_gameSessionNegotiationTce.set(gameSessionParameters);
						}
					}
				});

				gameSessionContainer->OnTunnelOpened = gameSessionContainer->service()->OnTunnelOpened.subscribe([wThat](std::shared_ptr<Stormancer::P2PTunnel> p2pTunnel)
				{
					if (auto that = wThat.lock())
					{
						GameSessionConnectionParameters gameSessionParameters;
						gameSessionParameters.isHost = false;
						gameSessionParameters.Endpoint = p2pTunnel->ip + ":" + std::to_string(p2pTunnel->port);

						that->OnTunnelOpened(gameSessionParameters);
						that->_gameSessionNegotiationTce.set(gameSessionParameters);
					}
				});

				gameSessionContainer->AllPlayerReady = gameSessionContainer->service()->OnAllPlayerReady.subscribe([wThat]()
				{
					if (auto that = wThat.lock())
					{
						that->OnAllPlayerReady();
					}
				});

				return gameSessionContainer;
			}
			else
			{
				throw std::runtime_error("Game session destroyed");
			}
		});
	}
}
