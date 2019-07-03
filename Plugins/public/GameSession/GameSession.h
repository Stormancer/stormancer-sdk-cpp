#pragma once

#include "stormancer/Tasks.h"
#include "stormancer/Packet.h"
#include "stormancer/Event.h"
#include "stormancer/ConnectionState.h"
#include "GameSession/GameSessionModels.h"

namespace Stormancer
{
	class IP2PScenePeer;

	class GameSession
	{
	public:
		virtual ~GameSession() = default;

		virtual pplx::task<GameSessionConnectionParameters> connectToGameSession(std::string token, std::string mapName = "", bool openTunnel = true, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
		virtual pplx::task<void> setPlayerReady(const std::string& data = "", pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		template<typename TServerResult, typename TClientResult>
		pplx::task<TServerResult> postResult(const TClientResult& clientResult, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			StreamWriter streamWriter = [clientResult](obytestream& stream)
			{
				Serializer serializer;
				serializer.serialize(stream, clientResult);
			};
			return postResult(streamWriter, ct)
				.then([](Packetisp_ptr packet)
			{
				Serializer serializer;
				TServerResult serverResult = serializer.deserializeOne<TServerResult>(packet->stream);
				return serverResult;
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

		virtual std::shared_ptr<Scene> scene() = 0;

		Event<std::shared_ptr<Scene>> onConnectingToScene;
		Event<std::shared_ptr<Scene>> onDisconnectingFromScene;

		/// <summary>
		/// Get the P2P Host peer for this Game Session.
		/// </summary>
		/// <remarks>
		/// Players in a game session are connected together through a star topology centered on a single player, the Host.
		/// The players who are not the Host are called Clients.
		/// </remarks>
		/// <returns>
		/// The <c>IP2PScenePeer</c> for the host of the session.
		/// If you are the host, or you are not yet connected to a game session, it will be nullptr.
		/// </returns>
		virtual std::shared_ptr<IP2PScenePeer> getSessionHost() const = 0;

		/// <summary>
		/// Check whether you are the P2P Host of the Game Session.
		/// </summary>
		/// <seealso cref="getSessionHost()"/>
		/// <returns>
		/// <c>true</c> if you are the host of the session ; <c>false</c> if you are a client, or if you are not connected to a game session.
		/// </returns>
		virtual bool isSessionHost() const = 0;

		/// <summary>
		/// Event that is triggered when a host migration happens.
		/// </summary>
		/// <remarks>
		/// Host migration is not currently supported.
		/// </remarks>
		Event<std::shared_ptr<IP2PScenePeer>> onSessionHostChanged;
	};
}
