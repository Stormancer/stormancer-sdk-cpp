#pragma once
#include <stormancer.h>
#include <string>

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

	enum class FleetSize
	{
		Unknown = -1,
		Small = 1,
		Medium = 2,
		Large = 3
	};

	enum class Readiness
	{
		Unknown = 0,
		Ready = 1,
		NotReady = 2
	};

	struct Player
	{
	public:
		
		std::string playerId;
		std::string pseudo;

		MSGPACK_DEFINE(playerId, pseudo);
	};

	struct MatchmakingRequest
	{
	public:
		std::map<std::string, std::string> profileIds;

		MSGPACK_DEFINE(profileIds);
	};

	struct MatchmakingResponse
	{
	public:
		std::string gameId;
		std::vector<Player> team1;
		std::vector<Player> team2;
		std::vector<std::string> optionalParameters;
	};

	struct ReadyVerificationRequest
	{
	public:
		std::map<std::string, Readiness> members;
		std::string matchId;
		Stormancer::int32 timeout;
		Stormancer::int32 membersCountReady;
		Stormancer::int32 membersCountTotal;
	};

	// Msgpack doesn't support using C++11 enum class so we use int32 instead in this temporary internal functions
	namespace Internal
	{
		struct MatchmakingResponse
		{
		public:
			std::string gameId;
			std::vector<Player> team1;
			std::vector<Player> team2;
			std::vector<std::string> optionalParameters;
			

			MSGPACK_DEFINE(gameId, team1, team2, optionalParameters);
		};

		struct ReadyVerificationRequest
		{
		public:
			std::map<std::string, Stormancer::int32> members;
			std::string matchId;
			Stormancer::int32 timeout;

			MSGPACK_DEFINE(members, matchId, timeout);
		};
	}

	class MatchmakingService
	{
	public:
		MatchmakingService(Stormancer::Scene* scene);
		~MatchmakingService();

		MatchmakingService(const MatchmakingService& other) = delete;
		MatchmakingService(const MatchmakingService&& other) = delete;
		MatchmakingService& operator=(const MatchmakingService&& other) = delete;

		void onMatchUpdate(std::function<void(MatchState)> callback);
		void onMatchParametersUpdate(std::function<void(MatchmakingRequest)> callback);
		void onMatchReadyUpdate(std::function<void(ReadyVerificationRequest)> callback);
		void onMatchFound(std::function<void(MatchmakingResponse)> callback);

		MatchState matchState() const;

		pplx::task<std::shared_ptr<Stormancer::Result<>>> findMatch(std::string provider, std::map<std::string, std::string> profileIds);

		void resolve(bool acceptMatch);

		void cancel();

	private:
		Stormancer::Scene* _scene = nullptr;
		Stormancer::IRpcService* _rpcService = nullptr;
		bool _isMatching = false;
		std::weak_ptr<Stormancer::ISubscription> _matchmakingSubscription;
		std::function<void(MatchState)> _onMatchUpdate;
		std::function<void(MatchmakingRequest)> _onMatchParametersUpdate;
		std::function<void(ReadyVerificationRequest)> _onMatchReadyUpdate;
		std::function<void(MatchmakingResponse)> _onMatchFound;
		MatchState _matchState = MatchState::Unknown;
		MatchmakingRequest _matchmakingRequest;
	};
};
