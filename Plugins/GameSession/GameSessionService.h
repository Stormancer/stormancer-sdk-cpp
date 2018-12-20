#pragma once
#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/IClient.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/Event.h"
#include "stormancer/Action.h"

namespace Stormancer
{
	///
	/// Const var
	const std::string GAMESESSION_P2P_SERVER_ID = "GameSession";

	///
	/// Forward declare
	struct GameSessionContainer;

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

	struct EndGameDto
	{
		Stormancer::uint64 score;
		std::string LeaderboardName;
		MSGPACK_DEFINE(score, LeaderboardName)
	};

	struct GameSessionResult
	{
		std::map<std::string, std::string> usersScore;
		MSGPACK_DEFINE(usersScore)
	};

	struct GameSessionConnectionParameters
	{
		bool isHost;
		std::string hostMap;
		std::string Endpoint;
		std::string ErrorMessage;
	};

	class GameSession : public std::enable_shared_from_this<GameSession>
	{
	public:
		GameSession(std::weak_ptr<IClient> client);

		pplx::task<GameSessionConnectionParameters> ConnectToGameSession(std::string token, std::string mapName, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> SetPlayerReady(std::string data, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<GameSessionResult> PostResult(EndGameDto gameSessioResult, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<void> DisconectFromGameSession(pplx::cancellation_token ct = pplx::cancellation_token::none());

		Event<void> OnAllPlayerReady;
		Event<GameSessionConnectionParameters> OnRoleRecieved;
		Event<GameSessionConnectionParameters> OnTunnelOpened;

	private:
		std::string _mapName;
		std::weak_ptr<IClient> _wClient;
		pplx::task<std::shared_ptr<GameSessionContainer>> _currentGameSession;
		pplx::task_completion_event<GameSessionConnectionParameters> _gameSessionNegotiationTce;

		pplx::task<std::shared_ptr<GameSessionContainer>> connectToGameSessionImpl(std::string token, pplx::cancellation_token ct);
		pplx::task<std::shared_ptr<GameSessionContainer>> getCurrentGameSession(pplx::cancellation_token ct);
		pplx::task<std::string> P2PTokenRequest(std::shared_ptr<GameSessionContainer> gameSessionContainer, pplx::cancellation_token ct);

		std::mutex _lock;
	};

	class GameSessionService : public std::enable_shared_from_this<GameSessionService>
	{
		friend GameSession;
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
		//std::function<void()> OnRoleReceived(std::function<void(std::string)> callback);

		// This functions need to be replace by action2 (event)
		//void OnTunnelOpened(std::function<void(std::shared_ptr<Stormancer::P2PTunnel>)> callback);
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
