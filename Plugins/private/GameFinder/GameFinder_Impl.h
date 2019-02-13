#pragma once

#include "stormancer/Event.h"
#include "GameFinder/GameFinderModels.h"
#include "GameFinder/GameFinder.h"
#include "stormancer/Event.h"

namespace Stormancer
{
	class AuthenticationService;
	struct GameFinderContainer;
	
	class GameFinder_Impl : public std::enable_shared_from_this<GameFinder_Impl>, public GameFinder
	{
	public:		
		GameFinder_Impl(std::weak_ptr<AuthenticationService> auth);		
		
		pplx::task<void> findGame(std::string gameFinder,const std::string &provider, std::string json) override;
		void cancel(std::string gameFinder) override;

		Action2<GameFinderStatusChangedEvent> gameFinderStateChanged;
		Action2<GameFoundEvent> gameFound;

		std::unordered_map<std::string, GameFinderStatusChangedEvent> getPendingFindGameStatus() override;
		
		pplx::task<void> connectToGameFinder(std::string gameFinderName) override;
		pplx::task<void> disconnectFromGameFinder(std::string gameFinderName) override;

		Event<GameFinderStatusChangedEvent>::Subscription subsribeGameFinderStateChanged(std::function<void(GameFinderStatusChangedEvent)> callback) override;
		Event<GameFoundEvent>::Subscription subsribeGameFound(std::function<void(GameFoundEvent)> callback)  override;
	private:
		
		pplx::task<std::shared_ptr<GameFinderContainer>> connectToGameFinderImpl(std::string gameFinderName);
		pplx::task<std::shared_ptr<GameFinderContainer>> getGameFinderContainer(std::string id);
		

		std::mutex _lock;
		std::unordered_map<std::string, pplx::task<std::shared_ptr<GameFinderContainer>>> _gameFinders;
		std::unordered_map<std::string, pplx::cancellation_token_source> _pendingFindGameRequests;
		std::weak_ptr<AuthenticationService> _auth;
	};
}