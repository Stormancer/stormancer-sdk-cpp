#pragma once
#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/Client.h"
#include "stormancer/RPC/RpcService.h"

namespace Stormancer
{
	const std::string GAMESESSION_P2P_SERVER_ID = "GameSession";

	struct SetResult
	{
	public:
		std::string winnerId;

		int winnerScore;
		int loserScore;

		MSGPACK_DEFINE(winnerId, winnerScore, loserScore);
	};

	struct GameResults
	{
	public:
		std::string winnerId;

		bool forfait;
		std::vector<SetResult> sets;

		MSGPACK_DEFINE(winnerId, forfait, sets);
	};

	struct MMRChanges
	{
	public:
		std::string WinnerId;
		int WinnerGain;


		std::string LoserId;
		int LoserGain;

		std::string winnerNewLeague;
		std::string loserNewLeague;

		MSGPACK_DEFINE(WinnerId, WinnerGain, LoserId, LoserGain, winnerNewLeague, loserNewLeague);
	};

	enum class PlayerStatus
	{
		NotConnected = 0,
		Connected = 1,
		Ready = 2,
		Faulted = 3,
		Disconnected = 4
	};

	struct SessionPlayer
	{
	public:
		SessionPlayer(std::string playerId, PlayerStatus status)
			: PlayerId(playerId), Status(status) {}
		std::string PlayerId;
		PlayerStatus Status;
	};

	struct ServerStartedMessage
	{
	public:
		std::string p2pToken;
		MSGPACK_DEFINE(p2pToken);
	};

	struct PlayerUpdate
	{
	public:
		std::string UserId;
		int Status;
		std::string Data;

		MSGPACK_DEFINE(UserId, Status, Data);
	};

	class GameSessionManager
	{
	public:
		GameSessionManager(Client* client);
		std::string currentGameSessionId;

		void setToken(std::string token);

		pplx::task<Scene_ptr> getCurrentGameSession();

	private:
		std::string token;
		Client* client;
	};

	class GameSessionService
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
		GameSessionService(Stormancer::Scene_ptr scene);

		pplx::task<std::string> waitServerReady(pplx::cancellation_token);

		std::vector<SessionPlayer> GetConnectedPlayers();

		std::function<void()> OnConnectedPlayerChanged(std::function<void(SessionPlayer, std::string)> callback);
		std::function<void()> OnRoleReceived(std::function<void(std::string)> callback);
		void OnTunnelOpened(std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)> callback);

		void OnP2PConnected(std::function<void(std::shared_ptr<Stormancer::P2PScenePeer>)> callback);

		void OnShutdownReceived(std::function<void(void)> callback);

		template<typename TOut, typename TIn>
		pplx::task<TOut> sendGameResults(TIn results)
		{
			auto rpc = _scene->dependencyResolver()->resolve<RpcService>();
			return rpc->rpc<TOut, TIn>("gamesession.postresults", results);
		}

		pplx::task<void> connect();
		pplx::task<void> reset();
		pplx::task<void> disconnect();

		bool shouldEstablishTunnel = true;

		Scene_ptr GetScene() {
			return this->_scene;
		}

		void ready(std::string data);
	private:
		void unsubscribeConnectedPlayersChanged(Action<SessionPlayerUpdateArg>::TIterator handle);
		void unsubscribeRoleReceived(Action<std::string>::TIterator handle);

	private:
		std::shared_ptr<P2PTunnel> _tunnel;
		Action<SessionPlayerUpdateArg> _onConnectedPlayersChanged;
		Action<std::string> _onRoleReceived;
		std::function<void(void)> _onShutdownReceived;
		std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)> _onTunnelOpened;
		std::function<void(std::shared_ptr<Stormancer::P2PScenePeer>)> _onConnectionOpened;
		Scene_ptr _scene;
		std::vector<SessionPlayer> _users;
		pplx::task_completion_event<std::string> _waitServerTce;
		std::shared_ptr<Stormancer::ILogger> _logger;

	};
}
