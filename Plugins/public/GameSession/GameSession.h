#pragma once

#include "stormancer/Packet.h"
#include "stormancer/Event.h"
#include "stormancer/ConnectionState.h"
#include "GameSession/GameSessionModels.h"

namespace Stormancer
{	
	class GameSession
	{
	public:
		virtual ~GameSession() = default;

		virtual pplx::task<GameSessionConnectionParameters> ConnectToGameSession(std::string token, std::string mapName, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		virtual pplx::task<void> SetPlayerReady(std::string data, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		
		template<typename TServerResult, typename TClientResult>
		pplx::task<TServerResult> PostResult(const TClientResult& clientResult, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			StreamWriter streamWriter = [&clientResult](obytestream& stream)
			{
				Serializer serializer;
				serializer.serialize(stream, clientResult);
			};
			return PostResult(streamWriter, ct)
				.then([](Packetisp_ptr packet)
			{
				Serializer serializer;
				TServerResult serverResult = serializer.deserializeOne<TServerResult>(packet->stream);
			});
		}

		virtual pplx::task<Packetisp_ptr> PostResult(const StreamWriter& streamWriter, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		
		virtual pplx::task<std::string> GetUserFromBearerToken(std::string token) = 0;
		virtual pplx::task<void> DisconectFromGameSession() = 0;

		virtual Event<void>::Subscription subscibeOnAllPlayerReady(std::function<void()> callback) = 0;
		virtual Event<GameSessionConnectionParameters>::Subscription subscibeOnRoleRecieved(std::function<void(GameSessionConnectionParameters)> callback) = 0;
		virtual Event<GameSessionConnectionParameters>::Subscription subscibeOnTunnelOpened(std::function<void(GameSessionConnectionParameters)> callback) = 0;
		virtual Event<ConnectionState>::Subscription subscribeOnGameSessionConnectionChange(std::function<void(ConnectionState)> callback) = 0;
		virtual Event<void>::Subscription subscribeOnShutdownRecieved(std::function<void()> callback) = 0;
	};
}
