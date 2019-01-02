#pragma once

#include "stormancer/RPC/Service.h"
#include "stormancer/Event.h"
#include "stormancer/Action.h"
#include "GameSessionModels.h"

namespace Stormancer
{
	///
	/// Const var
	const std::string GAMESESSION_P2P_SERVER_ID = "GameSession";

	///
	/// Forward declare
	class Scene;
	class P2PTunnel;

	class GameSessionService : public std::enable_shared_from_this<GameSessionService>
	{

	private:
		struct SessionPlayerUpdateArg
		{
		public:
			SessionPlayerUpdateArg(SessionPlayer player, std::string data)
				: sessionPlayer(player), data(data) {}

			SessionPlayer sessionPlayer;
			std::string data;
		};

	public:
		GameSessionService(std::weak_ptr<Scene> scene);
		~GameSessionService();

		void Initialize();
		pplx::task<void> InitializeTunnel(std::string p2pToken, pplx::cancellation_token ct);

		pplx::task<void> waitServerReady(pplx::cancellation_token);

		std::vector<SessionPlayer> GetConnectedPlayers();

		std::function<void()> OnConnectedPlayerChanged(std::function<void(SessionPlayer, std::string)> callback);

		void OnP2PConnected(std::function<void(std::shared_ptr<Stormancer::IP2PScenePeer>)> callback);
		void OnShutdownReceived(std::function<void(void)> callback);
		void OnConnectionFailure(std::function<void(std::string)> callback);

		Event<void> OnAllPlayerReady;
		Event<std::string> OnRoleReceived;
		Event<std::shared_ptr<Stormancer::P2PTunnel>> OnTunnelOpened;

		template<typename TOut, typename TIn>
		pplx::task<TOut> sendGameResults(TIn results, pplx::cancellation_token ct)
		{
			auto scene = _scene.lock();
			if (!scene)
			{
				return pplx::task_from_exception<TOut>(std::runtime_error("Scene deleted"));
			}

			auto rpc = scene->dependencyResolver()->resolve<RpcService>();
			return rpc->rpc<TOut, TIn>("gamesession.postresults", results);
		}

		pplx::task<std::string> P2PTokenRequest(pplx::cancellation_token ct);

		pplx::task<void> reset(pplx::cancellation_token ct);
		pplx::task<void> disconnect();

		bool shouldEstablishTunnel = true;

		std::weak_ptr<Scene> GetScene();

		void onDisconnecting();

		void ready(std::string data);

	private:

		void unsubscribeConnectedPlayersChanged(Action<SessionPlayerUpdateArg>::TIterator handle);

		pplx::cancellation_token linkTokenToDisconnection(pplx::cancellation_token tokenToLink);
		
		std::shared_ptr<P2PTunnel> _tunnel;
		Action<SessionPlayerUpdateArg> _onConnectedPlayersChanged;
		Action<std::string> _onConnectionFailure;
		std::function<void(void)> _onShutdownReceived;
		std::function<void(std::shared_ptr<Stormancer::IP2PScenePeer>)> _onConnectionOpened;
		pplx::task_completion_event<void> _waitServerTce;

		std::weak_ptr<Scene> _scene;
		std::vector<SessionPlayer> _users;
		std::shared_ptr<Stormancer::ILogger> _logger;
		bool _receivedP2PToken = false;
		pplx::cancellation_token_source _disconnectionCts;
	};
}
