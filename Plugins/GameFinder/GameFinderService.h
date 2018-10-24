#pragma once
#include <memory>
#include "stormancer/headers.h"
#include "stormancer/stormancer.h"
#include "GameFinderTypes.h"
#include "stormancer/Action2.h"

namespace Stormancer
{
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
		std::map<std::string, std::string> optionalParameters;

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

	namespace Internal
	{
		struct ReadyVerificationRequest
		{
			std::map<std::string, int32> members;
			std::string matchId;
			int32 timeout;

			MSGPACK_DEFINE(members, matchId, timeout);

			operator Stormancer::ReadyVerificationRequest();
		};
	}

	class GameFinderService : public std::enable_shared_from_this<GameFinderService>
	{
	public:

		GameFinderService(Scene_ptr scene);
		void initialize();
		~GameFinderService();
		GameFinderService(const GameFinderService& other) = delete;
		GameFinderService(const GameFinderService&& other) = delete;
		GameFinderService& operator=(const GameFinderService&& other) = delete;
		GameFinderStatus currentState() const;
		pplx::task<void> findMatch(const std::string &provider, const GameFinderRequest &mmRequest);
		pplx::task<void> findMatch(const std::string &provider, std::string json);
		void resolve(bool acceptMatch);
		void cancel();

		Action2<GameFinderStatus> GameFinderStatusUpdated;
		Action2<GameFinderResponse> GameFound;

	private:

		std::weak_ptr<Scene> _scene;
		std::weak_ptr<RpcService> _rpcService;

		pplx::cancellation_token_source _matchmakingCTS;

		GameFinderStatus _currentState = GameFinderStatus::Idle;
		Serializer _serializer;

		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "GameFinder";
	};
}