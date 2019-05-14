#pragma once

#include "stormancer/Tasks.h"
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

		virtual pplx::task<GameSessionConnectionParameters> connectToGameSession(std::string token, std::string mapName = "", pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		virtual pplx::task<void> setPlayerReady(const std::string& data = "", pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		template<typename TServerResult, typename TClientResult>
		pplx::task<TServerResult> postResult(const TClientResult& clientResult, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			StreamWriter streamWriter = [&clientResult](obytestream& stream)
			{
				Serializer serializer;
				serializer.serialize(stream, clientResult);
			};
			return postResult(streamWriter, ct)
				.then([](Packetisp_ptr packet)
			{
				Serializer serializer;
				TServerResult serverResult = serializer.deserializeOne<TServerResult>(packet->stream);
			});
		}

		virtual pplx::task<Packetisp_ptr> postResult(const StreamWriter& streamWriter, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		virtual pplx::task<std::string> getUserFromBearerToken(const std::string& token) = 0;
		virtual pplx::task<void> disconectFromGameSession() = 0;

		virtual Subscription subscibeOnAllPlayerReady(std::function<void()> callback) = 0;
		virtual Subscription subscibeOnRoleRecieved(std::function<void(GameSessionConnectionParameters)> callback) = 0;
		virtual Subscription subscibeOnTunnelOpened(std::function<void(GameSessionConnectionParameters)> callback) = 0;
		virtual Subscription subscribeOnGameSessionConnectionChange(std::function<void(ConnectionState)> callback) = 0;
		virtual Subscription subscribeOnShutdownRecieved(std::function<void()> callback) = 0;
		virtual Subscription subscribeOnPlayerChanged(std::function<void(SessionPlayer, std::string)> callback) = 0;
	};
}
