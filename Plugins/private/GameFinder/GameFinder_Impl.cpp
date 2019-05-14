#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameFinder_Impl.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/IClient.h"
#include "GameFinderService.h"

namespace Stormancer
{
	struct GameFinderContainer
	{
		~GameFinderContainer()
		{
			if (connectionStateChangedSubscription.is_subscribed())
			{
				connectionStateChangedSubscription.unsubscribe();
			}
		}

		//Keep game finder scene alive
		std::shared_ptr<Scene> scene;
		std::shared_ptr<GameFinderService> service()
		{
			return scene->dependencyResolver()->resolve<GameFinderService>();
		}

		Subscription gameFoundSubscription;
		Subscription gameFinderStateUpdatedSubscription;
		Subscription findGamefailedSubscription;
		rxcpp::subscription connectionStateChangedSubscription;
	};

	GameFinder_Impl::GameFinder_Impl(std::weak_ptr<AuthenticationService> auth)
	{
		_auth = auth;
	}

	std::unordered_map<std::string, GameFinderStatusChangedEvent> GameFinder_Impl::getPendingFindGameStatus()
	{
		std::unordered_map<std::string, GameFinderStatusChangedEvent> result;
		for (auto gameFinder : _gameFinders)
		{
			GameFinderStatusChangedEvent status;
			status.gameFinder = gameFinder.first;
			auto task = gameFinder.second;
			if (task.is_done())
			{

				auto container = task.get();
				status.status = container->service()->currentState();
			}
			else
			{
				status.status = GameFinderStatus::Loading;
			}
			result.emplace(gameFinder.first, status);
		}

		return result;
	}

	pplx::task<void> GameFinder_Impl::findGame(const std::string& gameFinder, const std::string &provider, const StreamWriter& streamWriter)
	{
		std::weak_ptr<GameFinder_Impl> wThat = this->shared_from_this();
		{
			std::lock_guard<std::mutex> lg(_lock);

			auto pendingRequest = _pendingFindGameRequests.find(gameFinder);
			if (pendingRequest != _pendingFindGameRequests.end())
			{
				return pplx::task_from_exception<void>(std::runtime_error(("A findGame request is already running for GameFinder '" + gameFinder + "'").c_str()));
			}
			_pendingFindGameRequests.emplace(gameFinder, pplx::cancellation_token_source{});
		}
		auto ct = _pendingFindGameRequests[gameFinder].get_token();

		return getGameFinderContainer(gameFinder)
			.then([provider, streamWriter, gameFinder, ct](std::shared_ptr<GameFinderContainer> gameFinderContainer)
		{
			if (ct.is_canceled())
			{
				pplx::cancel_current_task();
			}
			auto findGameTask = gameFinderContainer->service()->findGame(provider, streamWriter);
			ct.register_callback([gameFinderContainer] { gameFinderContainer->service()->cancel(); });
			return findGameTask;
		})
			.then([wThat, gameFinder](pplx::task<void> task)
		{
			if (auto that = wThat.lock())
			{
				std::lock_guard<std::mutex> lg(that->_lock);
				that->_pendingFindGameRequests.erase(gameFinder);
			}
			return task;
		});
	}

	pplx::task<std::shared_ptr<GameFinderContainer>> GameFinder_Impl::getGameFinderContainer(std::string id)
	{
		std::lock_guard<std::mutex> lg(_lock);
		auto it = _gameFinders.find(id);
		if (it != _gameFinders.end())
		{
			return (*it).second;
		}
		else
		{
			auto t = connectToGameFinderImpl(id);
			_gameFinders.emplace(id, t);
			return t;
		}
	}

	pplx::task<void> GameFinder_Impl::connectToGameFinder(const std::string& id)
	{
		return getGameFinderContainer(id).then([](std::shared_ptr<GameFinderContainer>) {});
	}

	pplx::task<std::shared_ptr<GameFinderContainer>> GameFinder_Impl::connectToGameFinderImpl(std::string gameFinderName)
	{
		auto auth = _auth.lock();

		if (!auth)
		{
			return pplx::task_from_exception<std::shared_ptr<GameFinderContainer>>(std::runtime_error("Authentication service destroyed."));
		}

		std::weak_ptr<GameFinder_Impl> wThat = this->shared_from_this();

		return auth->getSceneForService("stormancer.plugins.gamefinder", gameFinderName)
			.then([gameFinderName, wThat](pplx::task<std::shared_ptr<Scene>> task)
		{
			try
			{
				auto container = std::make_shared<GameFinderContainer>();
				container->scene = task.get();
				container->connectionStateChangedSubscription = container->scene->getConnectionStateChangedObservable().subscribe([wThat, gameFinderName](ConnectionState s)
				{
					if (auto that = wThat.lock())
					{
						if (s == ConnectionState::Disconnecting)
						{
							std::lock_guard<std::mutex> lg(that->_lock);
							auto it = that->_gameFinders.find(gameFinderName);
							if (it != that->_gameFinders.end())
							{
								that->_gameFinders.erase(it);
							}
						}
					}
				});
				auto service = container->service();
				container->gameFoundSubscription = service->GameFound.subscribe([wThat, gameFinderName](GameFinderResponse r)
				{
					if (auto that = wThat.lock())
					{
						GameFoundEvent ev;
						ev.gameFinder = gameFinderName;
						ev.data = r;
						that->gameFound(ev);
					}
				});
				container->gameFinderStateUpdatedSubscription = service->GameFinderStatusUpdated.subscribe([wThat, gameFinderName](GameFinderStatus s)
				{
					if (auto that = wThat.lock())
					{
						GameFinderStatusChangedEvent ev;
						ev.gameFinder = gameFinderName;
						ev.status = s;
						that->gameFinderStateChanged(ev);
					}
				});
				container->findGamefailedSubscription = service->FindGameRequestFailed.subscribe([wThat, gameFinderName](std::string reason)
				{
					if (auto that = wThat.lock())
					{
						FindGameFailedEvent ev;
						ev.gameFinder = gameFinderName;
						ev.reason = reason;
						that->findGameFailed(ev);
					}
				});
				return container;
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error("Failed to connect to game finder. sceneName=" + gameFinderName + " reason=" + ex.what());
			}
		})
			.then([wThat, gameFinderName](std::shared_ptr<GameFinderContainer> container)
		{
			auto that = wThat.lock();
			if (!that)
			{
				throw std::runtime_error("destroyed");
			}
			return container;
		})
			.then([wThat, gameFinderName](pplx::task<std::shared_ptr<GameFinderContainer>> task)
		{
			try
			{
				return task.get();
			}
			catch (std::exception&)
			{
				auto that = wThat.lock();
				if (that)
				{
					std::lock_guard<std::mutex> lg(that->_lock);
					that->_gameFinders.erase(gameFinderName);
				}
				throw;
			}
		});
	}

	pplx::task<void> GameFinder_Impl::disconnectFromGameFinder(const std::string& gameFinderName)
	{
		std::lock_guard<std::mutex> lg(this->_lock);
		auto it = _gameFinders.find(gameFinderName);
		if (it != _gameFinders.end())
		{
			auto containerTask = it->second;
			_gameFinders.erase(it);
			std::weak_ptr<GameFinder> wThat = this->shared_from_this();
			return containerTask.then([wThat, gameFinderName](std::shared_ptr<GameFinderContainer> gameFinder)
			{
				return gameFinder->scene->disconnect();
			});
		}
		return pplx::task_from_result();
	}

	Subscription GameFinder_Impl::subsribeGameFinderStateChanged(std::function<void(GameFinderStatusChangedEvent)> callback)
	{
		return gameFinderStateChanged.subscribe(callback);
	}

	Subscription GameFinder_Impl::subsribeGameFound(std::function<void(GameFoundEvent)> callback)
	{
		return gameFound.subscribe(callback);
	}

	Subscription GameFinder_Impl::subscribeFindGameFailed(std::function<void(FindGameFailedEvent)> callback)
	{
		return findGameFailed.subscribe(callback);
	}

	void GameFinder_Impl::cancel(const std::string& gameFinder)
	{
		std::lock_guard<std::mutex> lg(this->_lock);

		auto findGameRequest = _pendingFindGameRequests.find(gameFinder);
		if (findGameRequest != _pendingFindGameRequests.end())
		{
			findGameRequest->second.cancel();
		}
	}
}
