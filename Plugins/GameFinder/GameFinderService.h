#pragma once
#include <memory>
#include "stormancer/headers.h"
#include "stormancer/stormancer.h"
#include "GameFinderTypes.h"
#include "stormancer/Action2.h"

namespace Stormancer
{
	
	class GameFinderRequest
	{
	public:
		MSGPACK_DEFINE(profileIds, MapName)
		std::map<std::string, std::string> profileIds;
		std::string MapName;
	};

	struct PlayerDto
	{
		MSGPACK_DEFINE(steamId, playerId, steamName)
		u_int64 steamId;
		std::string playerId;
		std::string steamName;
	};

	struct TeamDto
	{
		MSGPACK_DEFINE(team)
		std::vector<PlayerDto> team;
	};

	struct GameFinderResponseDto
	{
		MSGPACK_DEFINE(gameToken, optionalParameters)
		std::string gameToken;	
		std::map<std::string,std::string> optionalParameters;
	};

	class PlayerProfile
	{
	public:
		MSGPACK_DEFINE(Id, PlayerId, Faction, Elo, League, Rank, MaxRank, UnlockedPortrait, SteamId, CurrentXP)
		std::string Id;
		std::string PlayerId;
		std::string Faction;
		u_int Elo;
		u_int League;
		u_int Rank;
		u_int MaxRank;
		std::vector<u_int> UnlockedPortrait;
		std::string SteamId;
		u_int CurrentXP;
	};

	class ProfileSummary
	{
	public:
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
			EloBO3)
			u_int AdmiralLevel;
		std::string ProfileName;
		std::string Difficulty;
		u_int Elo;
		std::string Faction;
		std::string Id;
		bool IsSolo;
		u_int Rank;
		u_int MaxRank;
		std::string SubFaction;
		bool IsElite;
		bool IsRanked;
		u_int League;
		u_int EloBO3;
	};

	class ReadyVerificationRequest
	{
	public:
		std::map<std::string, Readiness> members;
		std::string matchId;
		int32 timeout;
		int32 membersCountReady;
		int32 membersCountTotal;
	};

	namespace Internal
	{
		class ReadyVerificationRequest
		{
		public:
			MSGPACK_DEFINE(members, matchId, timeout)
				std::map<std::string, int32> members;
			std::string matchId;
			int32 timeout;


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
		std::shared_ptr<RpcService> _rpcService;
		
		pplx::cancellation_token_source _matchmakingCTS;
		
		GameFinderStatus _currentState = GameFinderStatus::Idle;
		Serializer _serializer;

		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "GameFinder";
	};
}