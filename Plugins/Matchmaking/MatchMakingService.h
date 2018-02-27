#pragma once

#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/RpcService.h"

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

	struct Player
	{
		std::string playerId;
		std::string pseudo;
		int64 rank;
		std::string division;

		MSGPACK_DEFINE(playerId, pseudo, rank, division);
	};

	struct MatchmakingResponse
	{
		std::string gameId;
		Player player1;
		Player player2;
		std::string hostId;
		std::vector<std::string> optionalParameters;

		MSGPACK_DEFINE(gameId, player1, player2, hostId, optionalParameters);
	};

	struct ReadyVerificationRequest
	{
		std::map<std::string, Readiness> members;
		std::string matchId;
		int32 timeout;
		int32 membersCountReady;
		int32 membersCountTotal;
	};

	// Msgpack doesn't support using C++11 enum class so we use int32 instead in this temporary internal functions
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

	class MatchmakingService
	{
	public:

#pragma region public_methods

		MatchmakingService(Scene_ptr scene);
		~MatchmakingService();
		MatchmakingService(const MatchmakingService& other) = delete;
		MatchmakingService(const MatchmakingService&& other) = delete;
		MatchmakingService& operator=(const MatchmakingService&& other) = delete;
		MatchState matchState() const;
		pplx::task<void> findMatch(std::string provider);
		void resolve(bool acceptMatch);
		void cancel();
		void onMatchUpdate(std::function<void(MatchState)> callback);
		void onMatchReadyUpdate(std::function<void(ReadyVerificationRequest)> callback);
		void onMatchFound(std::function<void(MatchmakingResponse)> callback);

#pragma endregion

	private:

#pragma region private_members

		Scene_ptr _scene;
		std::shared_ptr<RpcService> _rpcService;
		bool _isMatching = false;
		bool _hasSubscription = false;
		rxcpp::subscription _matchmakingSubscription;
		std::function<void(MatchState)> _onMatchUpdate;
		std::function<void(ReadyVerificationRequest)> _onMatchReadyUpdate;
		std::function<void(MatchmakingResponse)> _onMatchFound;
		MatchState _matchState = MatchState::Unknown;
		Serializer _serializer;

#pragma endregion
	};
};
