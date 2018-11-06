#pragma once

#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/service.h"

namespace Stormancer
{
	enum class MatchState
	{
		Unknown = -1,
		SearchStart = 0,
		CandidateFound = 1,
		WaitingPlayersReady = 2,
		Success = 3,
		Failed = 4,
		Canceled = 5
	};

	enum class Readiness
	{
		Unknown = 0,
		Ready = 1,
		NotReady = 2
	};

	struct MatchmakingRequest
	{


		MSGPACK_DEFINE_ARRAY();
	};

	struct Player
	{
		uint64 steamId;
		std::string playerId;
		std::string steamName;

		MSGPACK_DEFINE_ARRAY(steamId,playerId,steamName);
	};

	struct Team
	{
		std::vector<Player> players;
		MSGPACK_DEFINE_ARRAY(players);
	};

	struct MatchmakingResponse
	{
		std::string gameToken;

		std::vector<Team> teams;

		std::unordered_map<std::string, std::string> optionalParameters;

		MSGPACK_DEFINE_ARRAY(gameToken, teams, optionalParameters);
	};

	struct ReadyVerificationRequest
	{
		std::map<std::string, Readiness> members;
		std::string matchId;
		int32 timeout;
		int32 membersCountReady;

		MSGPACK_DEFINE_ARRAY(members, matchId, timeout);
	};

	class MatchmakingService
	{
	public:

#pragma region public_methods

		MatchmakingService(std::shared_ptr<Scene> scene);
		~MatchmakingService();

		MatchmakingService(const MatchmakingService& other) = delete;
		MatchmakingService(const MatchmakingService&& other) = delete;
		MatchmakingService& operator=(const MatchmakingService&& other) = delete;

		MatchState matchState() const;
		pplx::task<void> findMatch(const std::string& provider, const MatchmakingRequest& mmRequest);
		void resolve(bool acceptMatch);
		void cancel();
		void onMatchUpdate(std::function<void(MatchState)> callback);
		void onMatchReadyUpdate(std::function<void(ReadyVerificationRequest)> callback);
		void onMatchFound(std::function<void(MatchmakingResponse)> callback);

#pragma endregion

	private:

#pragma region private_members

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;
		bool _isMatching = false;
		pplx::cancellation_token_source _matchmakingCTS;
		std::function<void(MatchState)> _onMatchUpdate;
		std::function<void(ReadyVerificationRequest)> _onMatchReadyUpdate;
		std::function<void(MatchmakingResponse)> _onMatchFound;
		MatchState _matchState = MatchState::Unknown;
		Serializer _serializer;
		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "Matchmaking";

#pragma endregion
	};
}

MSGPACK_ADD_ENUM(Stormancer::Readiness);
