#pragma once
#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"
#include <string>
#include <vector>
#include <map>

namespace Stormancer
{
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
}