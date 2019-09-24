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
		std::string winnerId;
		int winnerGain;


		std::string loserId;
		int loserGain;

		std::string winnerNewLeague;
		std::string loserNewLeague;

		MSGPACK_DEFINE(winnerId, winnerGain, loserId, loserGain, winnerNewLeague, loserNewLeague);
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
		SessionPlayer(std::string playerId, PlayerStatus status, bool isHost = false)
			: playerId(playerId)
			, status(status)
			, isHost(isHost)
		{
		}

		std::string playerId;
		PlayerStatus status;
		bool isHost;
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
		std::string userId;
		int status;
		std::string data;
		bool isHost;

		MSGPACK_DEFINE(userId, status, data, isHost);
	};

	struct EndGameDto
	{
		Stormancer::uint64 score;
		std::string leaderboardName;
		MSGPACK_DEFINE(score, leaderboardName)
	};

	struct GameSessionConnectionParameters
	{
		bool isHost;
		std::string hostMap;
		std::string endpoint;
	};
}
