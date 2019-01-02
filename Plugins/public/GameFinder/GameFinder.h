#pragma once

#include "stormancer/Event.h"
#include "GameFinder/GameFinderModels.h"

namespace Stormancer
{
	class GameFinder
	{
	public:

		virtual pplx::task<void> findGame(std::string gameFinder, const std::string &provider, std::string json) = 0;
		virtual void cancel(std::string gameFinder) = 0;

		virtual std::unordered_map<std::string, GameFinderStatusChangedEvent> getPendingFindGameStatus() = 0;

		virtual pplx::task<void> connectToGameFinder(std::string gameFinderName) = 0;
		virtual pplx::task<void> disconnectFromGameFinder(std::string gameFinderName) = 0;

		virtual Event<GameFinderStatusChangedEvent>::Subscription subsribeGameFinderStateChanged(std::function<void(GameFinderStatusChangedEvent)> callback) = 0;
		virtual Event<GameFoundEvent>::Subscription subsribeGameFound(std::function<void(GameFoundEvent)> callback) = 0;
	};
}
