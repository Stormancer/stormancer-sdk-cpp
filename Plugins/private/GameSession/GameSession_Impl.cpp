#include "GameSession_Impl.h"
#include "stormancer/Scene.h"
#include "GameSessionService.h"
#include "stormancer/P2P/P2PTunnel.h"
#include "stormancer/IClient.h"
#include "stormancer/Utilities/TaskUtilities.h"
#include <chrono>

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

		pplx::task<GameSessionConnectionParameters> gameSessionReadyTask;

		Event<void>::Subscription AllPlayerReady;
		Event<std::string>::Subscription OnRoleRecieved;
		Event<std::shared_ptr<Stormancer::P2PTunnel>>::Subscription OnTunnelOpened;
		Event<void>::Subscription OnShutdownRecieved;

		rxcpp::subscription SceneConnectionState;
		~GameSessionContainer()
		{
			auto logger = scene->dependencyResolver()->resolve<ILogger>();
			auto sceneId = scene->id();
			scene->disconnect().then([logger, sceneId](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& ex)
				{
					if (logger)
					{
						logger->log(LogLevel::Warn, "GameSession", "Error disconnecting scene " + sceneId, ex.what());
					}
				}
			});
			if (logger)
			{
				logger->log(LogLevel::Info, "GameSession", "Destroyed", sceneId);
			}
		}
	};

	GameSession_Impl::GameSession_Impl(std::weak_ptr<IClient> client)
		: _wClient(client)
		, _currentGameSession(pplx::task_from_result<std::shared_ptr<GameSessionContainer>>(nullptr))
	{
	}

	// Add cancelation token for timeout or cancel.
	pplx::task<GameSessionConnectionParameters> GameSession_Impl::ConnectToGameSession(std::string token, std::string mapName, pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lg(_lock);
		
		if (!_currentGameSession.is_done())
		{
			throw pplx::task_from_exception<GameSessionConnectionParameters>(std::runtime_error("Game session connection is in pending. Cannot connect to an other game session"));
		}

		if (token.empty())
		{
			return pplx::task_from_exception<GameSessionConnectionParameters>(std::runtime_error("Game session can't be connected without token"));
		}

		// Client should never be null here
		auto dispatcher = _wClient.lock()->dependencyResolver()->resolve<IActionDispatcher>();
		_mapName = mapName;
		std::weak_ptr<GameSession_Impl> wThat = this->shared_from_this();

		// Disconnect from gameSession if the current ct equal the pending CT.
		_currentGameSession = DisconectFromGameSession()
			.then([wThat, token,ct]()
		{
			if (auto that = wThat.lock())
			{
				return that->connectToGameSessionImpl(token, ct);
			}
			else
			{
				throw std::runtime_error("Game session destroyed");
			}
		}, ct)
			.then([wThat, ct](std::shared_ptr<GameSessionContainer> gameSessionContainer)
		{
			if (auto that = wThat.lock())
			{
				return that->P2PTokenRequest(gameSessionContainer, ct)
					.then([gameSessionContainer, ct](pplx::task<std::string> task)
				{
					auto service = gameSessionContainer->service();
					auto logger = gameSessionContainer->scene->dependencyResolver()->resolve<ILogger>();
					try
					{
						auto token = task.get();
						return service->InitializeTunnel(token, ct);
					}
					catch (std::exception& e)
					{
						e.what();
						throw std::runtime_error("Cannot get p2pToken");
					}
				}, ct)
					.then([gameSessionContainer](pplx::task<void> t) {
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
				}, ct);
			}
			else
			{
				throw std::runtime_error("Game session deleted");
			}
		}, ct);

		return _currentGameSession
			.then([](std::shared_ptr<GameSessionContainer> c)
		{
			return c->gameSessionReadyTask;
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
					return that->DisconectFromGameSession().then([wThat, ptrEx](pplx::task<void> task) -> GameSessionConnectionParameters
					{
						try
						{
							task.get();								
						}
						catch (const std::exception& ex)
						{
							if (auto that = wThat.lock())
							{
								auto logger = that->_wClient.lock()->dependencyResolver()->resolve<ILogger>();
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


	pplx::task<void> GameSession_Impl::SetPlayerReady(std::string data, pplx::cancellation_token ct)
	{
		return this->getCurrentGameSession(ct)
			.then([data](std::shared_ptr<GameSessionContainer> container)
		{
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

	pplx::task<Packetisp_ptr> GameSession_Impl::PostResult(const StreamWriter& streamWriter, pplx::cancellation_token ct)
	{
		return getCurrentGameSession(ct)
			.then([streamWriter, ct](std::shared_ptr<GameSessionContainer> container)
		{
			if (container)
			{
				std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<GameSessionService>();
				return gameSessionService->sendGameResults(streamWriter, ct);
			}
			else
			{
				throw std::runtime_error("Not connected to any game session");
			}
		});
	}

	pplx::task<std::string> GameSession_Impl::GetUserFromBearerToken(std::string token)
	{
		return this->getCurrentGameSession()
			.then([token](std::shared_ptr<GameSessionContainer> container)
		{
			if (container)
			{
				std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<Stormancer::GameSessionService>();
				return gameSessionService->GetUserFromBearerToken(token);
			}
			else
			{
				throw std::runtime_error("Not connected to any game session");
			}
		});
	}

	pplx::task<void> GameSession_Impl::DisconectFromGameSession()
	{
		std::weak_ptr<GameSession> wThat = this->shared_from_this();
		// catch err
		return this->getCurrentGameSession()
			.then([wThat](pplx::task<std::shared_ptr<GameSessionContainer>> task)
		{
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

	Event<void>::Subscription GameSession_Impl::subscibeOnAllPlayerReady(std::function<void()> callback)
	{
		return OnAllPlayerReady.subscribe(callback);
	}

	Event<GameSessionConnectionParameters>::Subscription GameSession_Impl::subscibeOnRoleRecieved(std::function<void(GameSessionConnectionParameters)> callback)
	{
		return OnRoleRecieved.subscribe(callback);
	}

	Event<GameSessionConnectionParameters>::Subscription GameSession_Impl::subscibeOnTunnelOpened(std::function<void(GameSessionConnectionParameters)> callback)
	{
		return OnTunnelOpened.subscribe(callback);
	}

	Event<Stormancer::ConnectionState>::Subscription GameSession_Impl::subscribeOnGameSessionConnectionChange(std::function<void(ConnectionState)> callback)
	{
		return OnGameSessionConnectionChange.subscribe(callback);
	}

	Stormancer::Event<void>::Subscription GameSession_Impl::subscribeOnShutdownRecieved(std::function<void()> callback)
	{
		return _onShutdownReceived.subscribe(callback);
	}

	pplx::task<std::string> GameSession_Impl::P2PTokenRequest(std::shared_ptr<GameSessionContainer> container, pplx::cancellation_token ct)
	{
		std::weak_ptr<GameSession> wThat = this->shared_from_this();
		if (container)
		{
			std::shared_ptr<Stormancer::GameSessionService> gameSessionService = container->scene->dependencyResolver()->resolve<Stormancer::GameSessionService>();
			return gameSessionService->P2PTokenRequest(ct);
		}
		else
		{
			throw(std::runtime_error("GameSession doesn't exist"));
		}
	}

	pplx::task<std::shared_ptr<GameSessionContainer>> GameSession_Impl::getCurrentGameSession(pplx::cancellation_token ct)
	{
		// Use a continuation to "inject" the ct
		return _currentGameSession
			.then([](std::shared_ptr<GameSessionContainer> gamesession)
		{
			return gamesession;
		}, ct);
	}

	pplx::task<std::shared_ptr<GameSessionContainer>> GameSession_Impl::connectToGameSessionImpl(std::string token, pplx::cancellation_token ct)
	{
		std::weak_ptr<GameSession_Impl> wThat = this->shared_from_this();
		return _wClient.lock()->connectToPrivateScene(token, IClient::SceneInitializer{}, ct)
			.then([wThat](std::shared_ptr<Scene> scene)
		{
			if (auto that = wThat.lock())
			{
				auto gameSessionContainer = std::make_shared<GameSessionContainer>();
				gameSessionContainer->scene = scene;

				that->OnGameSessionConnectionChange(ConnectionState::Connected);

				pplx::task_completion_event<GameSessionConnectionParameters> sessionReadyTce;
				gameSessionContainer->gameSessionReadyTask = pplx::create_task(sessionReadyTce);

				gameSessionContainer->SceneConnectionState = scene->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state)
				{
					if (state == ConnectionState::Disconnected)
					{
						if (auto that = wThat.lock())
						{
							if (auto client = that->_wClient.lock())
							{
								pplx::create_task([wThat, state]() {
									if (auto that = wThat.lock())
									{
										that->OnGameSessionConnectionChange(state);
									}
								}, client->dependencyResolver()->resolve<IActionDispatcher>());
							}
							that->_currentGameSession = pplx::task_from_result<std::shared_ptr<GameSessionContainer>>(nullptr);
						}
					}
				});

				gameSessionContainer->OnRoleRecieved = gameSessionContainer->service()->OnRoleReceived.subscribe([wThat, sessionReadyTce](std::string role)
				{
					if (auto that = wThat.lock())
					{
						if (role == "HOST")
						{
							GameSessionConnectionParameters gameSessionParameters;
							gameSessionParameters.isHost = role == "HOST" ? true : false;
							gameSessionParameters.Endpoint = that->_mapName;
							that->OnRoleRecieved(gameSessionParameters);
							sessionReadyTce.set(gameSessionParameters);
						}
					}
				});

				gameSessionContainer->OnTunnelOpened = gameSessionContainer->service()->OnTunnelOpened.subscribe([wThat, sessionReadyTce](std::shared_ptr<Stormancer::P2PTunnel> p2pTunnel)
				{
					if (auto that = wThat.lock())
					{
						GameSessionConnectionParameters gameSessionParameters;
						gameSessionParameters.isHost = false;
						gameSessionParameters.Endpoint = p2pTunnel->ip + ":" + std::to_string(p2pTunnel->port);

						that->OnTunnelOpened(gameSessionParameters);
						sessionReadyTce.set(gameSessionParameters);
					}
				});

				gameSessionContainer->AllPlayerReady = gameSessionContainer->service()->OnAllPlayerReady.subscribe([wThat]()
				{
					if (auto that = wThat.lock())
					{
						that->OnAllPlayerReady();
					}
				});

				gameSessionContainer->OnShutdownRecieved = gameSessionContainer->service()->OnShutdownReceived.subscribe([wThat]()
				{
					if (auto that = wThat.lock())
					{
						that->_onShutdownReceived();
					}
				});

				return gameSessionContainer;
			}
			else
			{
				throw std::runtime_error("Game session destroyed");
			}
		},ct);
	}
}
