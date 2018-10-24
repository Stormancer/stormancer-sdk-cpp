#include "GameFinderManager.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/Client.h"
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
			return scene->dependencyResolver().lock()->resolve<GameFinderService>();
		}

		Action2<GameFinderResponse>::Subscription gameFoundSubscription;
		Action2<GameFinderStatus>::Subscription gameFinderStateUpdatedSubscription;
		rxcpp::subscription connectionStateChangedSubscription;
	};


	GameFinder::GameFinder(std::weak_ptr<AuthenticationService> auth)
	{
		_auth = auth;
	}

	std::unordered_map<std::string, GameFinderStatusChangedEvent> GameFinder::getPendingFindGameStatus()
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

	pplx::task<void> GameFinder::findGame(std::string gameFinder, const std::string &provider, std::string json)
	{
		std::weak_ptr<GameFinder> wThat = this->shared_from_this();

		return getGameFinderContainer(gameFinder).then([provider, json](std::shared_ptr<GameFinderContainer> c) {

			return c->service()->findMatch(provider, json);

		});
	}

	pplx::task<std::shared_ptr<GameFinderContainer>> GameFinder::getGameFinderContainer(std::string id)
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

	pplx::task<void> GameFinder::connectToGameFinder(std::string id)
	{
		return connectToGameFinderImpl(id).then([](std::shared_ptr<GameFinderContainer>) {});
	}

	pplx::task<std::shared_ptr<GameFinderContainer>> GameFinder::connectToGameFinderImpl(std::string gameFinderName)
	{
		auto auth = _auth.lock();

		if (!auth)
		{
			return pplx::task_from_exception<std::shared_ptr<GameFinderContainer>>(std::runtime_error("Authentication service destroyed."));
		}

		std::weak_ptr<GameFinder> wThat = this->shared_from_this();

		return auth->getSceneForService("stormancer.plugins.matchmaking", gameFinderName).then([gameFinderName, wThat](pplx::task<Scene_ptr> task) {

			try
			{

				auto container = std::make_shared<GameFinderContainer>();
				container->scene = task.get();
				container->connectionStateChangedSubscription = container->scene->getConnectionStateChangedObservable().subscribe([wThat, gameFinderName](ConnectionState s) {


					if (auto that = wThat.lock())
						if (s == ConnectionState::Disconnecting) 
						{

							std::lock_guard<std::mutex> lg(that->_lock);
							auto it = that->_gameFinders.find(gameFinderName);
							if (it != that->_gameFinders.end())
							{

								that->_gameFinders.erase(it);
							}
						}
				});
				container->gameFoundSubscription = container->service()->GameFound.subscribe([wThat, gameFinderName](GameFinderResponse r) {
					if (auto that = wThat.lock())
					{
						gameFoundEvent ev;
						ev.gameFinder = gameFinderName;
						ev.data = r;
						that->gameFound(ev);
					}
				});
				container->gameFinderStateUpdatedSubscription = container->service()->GameFinderStatusUpdated.subscribe([wThat, gameFinderName](GameFinderStatus s) {
					if (auto that = wThat.lock())
					{
						GameFinderStatusChangedEvent ev;
						ev.gameFinder = gameFinderName;
						ev.status = s;
						that->gameFinderStateChanged(ev);
					}
				});
				return container;
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error("Failed to connect to game finder. sceneName=" + gameFinderName + " reason=" + ex.what());
			}

		}).then([wThat, gameFinderName](std::shared_ptr<GameFinderContainer> container)
		{

			auto that = wThat.lock();

			if (!that)
			{
				throw std::runtime_error("destroyed");
			}

			return container;
		}).then([wThat, gameFinderName](pplx::task<std::shared_ptr<GameFinderContainer>> task)
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


	pplx::task<void> GameFinder::disconnectFromGameFinder(std::string gameFinderName)
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



	void GameFinder::cancel(std::string gameFinder)
	{
		std::lock_guard<std::mutex> lg(this->_lock);
		
		auto it = _gameFinders.find(gameFinder);
		if (it != _gameFinders.end())
		{

		}

	}



}