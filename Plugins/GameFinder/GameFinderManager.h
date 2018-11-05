#pragma once
#include "stormancer/Event.h"
#include "GameFinderTypes.h"

namespace Stormancer
{
	class AuthenticationService;
	struct GameFinderContainer;

	struct GameFinderStatusChangedEvent
	{
		GameFinderStatus status;
		std::string gameFinder;
	};
	struct gameFoundEvent
	{
		GameFinderResponse data;
		std::string gameFinder;
	};
	
	class GameFinder : public std::enable_shared_from_this<GameFinder>
	{
	public:		
		GameFinder(std::weak_ptr<AuthenticationService> auth);

		
		pplx::task<void> findGame(std::string gameFinder,const std::string &provider, std::string json);
		void cancel(std::string gameFinder);

		Event<GameFinderStatusChangedEvent> gameFinderStateChanged;
		Event<gameFoundEvent> gameFound;

		std::unordered_map<std::string, GameFinderStatusChangedEvent> getPendingFindGameStatus();
		
		pplx::task<void> connectToGameFinder(std::string gameFinderName);
		pplx::task<void> disconnectFromGameFinder(std::string gameFinderName);
	private:

		
		
		pplx::task<std::shared_ptr<GameFinderContainer>> connectToGameFinderImpl(std::string gameFinderName);
		pplx::task<std::shared_ptr<GameFinderContainer>> getGameFinderContainer(std::string id);
		

		std::mutex _lock;
		std::unordered_map<std::string, pplx::task<std::shared_ptr<GameFinderContainer>>> _gameFinders;
		std::weak_ptr<AuthenticationService> _auth;
	};



}