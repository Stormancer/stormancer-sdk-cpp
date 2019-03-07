#pragma once
#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include <unordered_map>
#include <string>
#include <map>
#include <vector>

namespace Stormancer
{
	enum class Readiness
	{
		Unknown = 0,
		Ready = 1,
		NotReady = 2
	};

	enum class GameFinderStatus
	{
		
		Idle = -1,
		
		Searching = 0,
		CandidateFound = 1,
		WaitingPlayersReady = 2,
		Success = 3,
		Failed = 4,
		Canceled = 5,
		Loading = 6
	};

	struct GameFinderResponse
	{

		std::string connectionToken;
		std::unordered_map<std::string, std::string> optionalParameters;
	};
	
	struct GameFinderStatusChangedEvent
	{
		GameFinderStatus status;
		std::string gameFinder;
	};
	
	struct GameFoundEvent
	{
		GameFinderResponse data;
		std::string gameFinder;
	};

	struct FindGameFailedEvent
	{
		std::string reason;
		std::string gameFinder;
	};

	struct GameFinderRequest
	{
		std::map<std::string, std::string> profileIds;
		std::string MapName;

		MSGPACK_DEFINE(profileIds, MapName);
	};

	struct PlayerDto
	{
		uint64 steamId;
		std::string playerId;
		std::string steamName;

		MSGPACK_DEFINE(steamId, playerId, steamName);
	};

	struct TeamDto
	{
		std::vector<PlayerDto> team;

		MSGPACK_DEFINE(team)
	};

	struct GameFinderResponseDto
	{
		std::string gameToken;
		std::unordered_map<std::string, std::string> optionalParameters;

		MSGPACK_DEFINE(gameToken, optionalParameters);
	};

	struct PlayerProfile
	{
		std::string Id;
		std::string PlayerId;
		std::string Faction;
		uint32 Elo;
		uint32 League;
		uint32 Rank;
		uint32 MaxRank;
		std::vector<uint32> UnlockedPortrait;
		std::string SteamId;
		uint32 CurrentXP;

		MSGPACK_DEFINE(Id, PlayerId, Faction, Elo, League, Rank, MaxRank, UnlockedPortrait, SteamId, CurrentXP);
	};

	struct ProfileSummary
	{
		uint32 AdmiralLevel;
		std::string ProfileName;
		std::string Difficulty;
		uint32 Elo;
		std::string Faction;
		std::string Id;
		bool IsSolo;
		uint32 Rank;
		uint32 MaxRank;
		std::string SubFaction;
		bool IsElite;
		bool IsRanked;
		uint32 League;
		uint32 EloBO3;

		MSGPACK_DEFINE(
			AdmiralLevel,
			ProfileName,
			Difficulty,
			Elo,
			Faction,
			Id,
			IsSolo,
			Rank,
			MaxRank,
			SubFaction,
			IsElite,
			IsRanked,
			League,
			EloBO3
		);
	};

	struct ReadyVerificationRequest
	{
		std::map<std::string, Readiness> members;
		std::string matchId;
		int32 timeout;
		int32 membersCountReady;
		int32 membersCountTotal;
	};
}