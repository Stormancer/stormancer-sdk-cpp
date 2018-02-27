#pragma once
#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/RpcService.h"

namespace Stormancer
{
	struct GameServerInformations
	{
		std::string Ip;
		int Port;

		MSGPACK_DEFINE(Ip, Port);
	};

	struct GameResults
	{
		std::string winnerId;
		int winnerScore;
		int loserScore;

		MSGPACK_DEFINE(winnerId, winnerScore, loserScore);
	};

	struct MatchConsequences
	{
		std::string WinnerId;
		int WinnerGain;
		std::string LoserId;
		int LoserGain;

		MSGPACK_DEFINE(WinnerId, WinnerGain, LoserId, LoserGain);
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
		std::string PlayerId;
		PlayerStatus Status;
	};

	struct PlayerUpdate
	{
		std::string UserId;
		int Status;
		std::string FaultReason;

		MSGPACK_DEFINE(UserId, Status, FaultReason);
	};

	class GameSessionService
	{
	public:

#pragma region public_methods

		GameSessionService(Scene_ptr scene);

		pplx::task<GameServerInformations> waitServerReady(pplx::cancellation_token);

		std::vector<SessionPlayer> getConnectedPlayers();

		std::function<void()> onConnectedPlayerChanged(std::function<void(SessionPlayer)> callback);

		template<typename TOut, typename TIn>
		pplx::task<TOut> sendGameResults(TIn results)
		{
			auto rpc = _scene->dependencyResolver()->resolve<RpcService>();
			return rpc->rpc<TOut, TIn>("gamesession.postresults", results);
		}

		pplx::task<void> connect();
		pplx::task<void> reset();

		void ready();

#pragma endregion

	private:

#pragma region private_methods

		void unsubscribeConnectedPlayersChanged(Action<SessionPlayer>::TIterator handle);

#pragma endregion

#pragma region private_members

		Action<SessionPlayer> _onConnectedPlayersChanged;
		Scene_ptr _scene;
		std::vector<SessionPlayer> _users;
		pplx::task_completion_event<GameServerInformations> _waitServerTce;

#pragma endregion
	};
}
