#pragma once
#include "stormancer/Action2.h"
#include "GameFinderTypes.h"

namespace Stormancer
{
	class Client;
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
		GameFinder(Client* scene);

		
		pplx::task<void> findGame(std::string gameFinder,const std::string &provider, std::string json);
		void cancel(std::string gameFinder);

		Action2<GameFinderStatusChangedEvent> gameFinderStateChanged;
		Action2<gameFoundEvent> gameFound;

		std::unordered_map<std::string, GameFinderStatusChangedEvent> getPendingFindGameStatus();
		
		pplx::task<std::shared_ptr<GameFinderContainer>> connectToGameFinder(std::string gameFinderName);
		pplx::task<void> disconnectFromGameFinder(std::string gameFinderName);
	private:

		
		
		
		pplx::task<std::shared_ptr<GameFinderContainer>> getGameFinderContainer(std::string id);
		

		std::mutex _lock;
		std::unordered_map<std::string, pplx::task<std::shared_ptr<GameFinderContainer>>> _gameFinders;
		Client* _client;
	};



}