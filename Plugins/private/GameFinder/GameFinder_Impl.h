#pragma once

#include "stormancer/Event.h"
#include "GameFinder/GameFinderModels.h"
#include "GameFinder/GameFinder.h"
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"

namespace Stormancer
{
	class AuthenticationService;
	struct GameFinderContainer;
	
	class GameFinder_Impl : public std::enable_shared_from_this<GameFinder_Impl>, public GameFinder
	{
	public:

		GameFinder_Impl(std::weak_ptr<AuthenticationService> auth);		
		
		pplx::task<void> findGame(const std::string& gameFinder, const std::string& provider, const StreamWriter& streamWriter) override;
		void cancel(const std::string& gameFinder) override;

		Event<GameFinderStatusChangedEvent> gameFinderStateChanged;
		Event<GameFoundEvent> gameFound;
		Event<FindGameFailedEvent> findGameFailed;

		std::unordered_map<std::string, GameFinderStatusChangedEvent> getPendingFindGameStatus() override;
		
		pplx::task<void> connectToGameFinder(const std::string& gameFinderName) override;
		pplx::task<void> disconnectFromGameFinder(const std::string& gameFinderName) override;

		Subscription subsribeGameFinderStateChanged(std::function<void(GameFinderStatusChangedEvent)> callback) override;
		Subscription subsribeGameFound(std::function<void(GameFoundEvent)> callback)  override;
		Subscription subscribeFindGameFailed(std::function<void(FindGameFailedEvent)> callback) override;

	private:
		
		pplx::task<std::shared_ptr<GameFinderContainer>> connectToGameFinderImpl(std::string gameFinderName);
		pplx::task<std::shared_ptr<GameFinderContainer>> getGameFinderContainer(std::string id);
		
		std::mutex _lock;
		std::unordered_map<std::string, pplx::task<std::shared_ptr<GameFinderContainer>>> _gameFinders;
		std::unordered_map<std::string, pplx::cancellation_token_source> _pendingFindGameRequests;
		std::weak_ptr<AuthenticationService> _auth;
	};
}
