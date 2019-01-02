#pragma once

#include "stormancer/Event.h"
#include "GameSession/GameSessionModels.h"

namespace Stormancer
{	
	class GameSession
	{
	public:
		virtual pplx::task<GameSessionConnectionParameters> ConnectToGameSession(std::string token, std::string mapName, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		virtual pplx::task<void> SetPlayerReady(std::string data, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		virtual pplx::task<GameSessionResult> PostResult(EndGameDto gameSessioResult, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		virtual pplx::task<void> DisconectFromGameSession() = 0;

		virtual Event<void>::Subscription subscibeOnAllPlayerReady(std::function<void()> callback) = 0;
		virtual Event<GameSessionConnectionParameters>::Subscription subscibeOnRoleRecieved(std::function<void(GameSessionConnectionParameters)> callback) = 0;
		virtual Event<GameSessionConnectionParameters>::Subscription subscibeOnTunnelOpened(std::function<void(GameSessionConnectionParameters)> callback) = 0;
	};
}