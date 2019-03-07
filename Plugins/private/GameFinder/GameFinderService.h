#pragma once
#include <memory>

#include "GameFinder/GameFinderModels.h"
#include "stormancer/Event.h"
#include "stormancer/scene.h"

namespace Stormancer
{
	class RpcService;
	class ILogger;

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

		GameFinderService(std::shared_ptr<Scene> scene);
		void initialize();
		~GameFinderService();
		GameFinderService(const GameFinderService& other) = delete;
		GameFinderService(const GameFinderService&& other) = delete;
		GameFinderService& operator=(const GameFinderService&& other) = delete;
		GameFinderStatus currentState() const;
		pplx::task<void> findMatch(const std::string &provider, const GameFinderRequest &mmRequest);
		pplx::task<void> findMatch(const std::string &provider, std::string json);
		void resolve(bool acceptMatch);
		// If cancel() is called very shortly after findMatch(), there might be a race condition.
		// It can be prevented by waiting for the first GameFinderStatusUpdated event received after calling findMatch() before calling cancel().
		void cancel();

		Event<GameFinderStatus> GameFinderStatusUpdated;
		Event<GameFinderResponse> GameFound;
		Event<std::string> FindGameRequestFailed;

	private:

		template<typename T>
		pplx::task<void> findMatchInternal(const std::string& provider, T data);

		std::weak_ptr<Scene> _scene;
		std::weak_ptr<RpcService> _rpcService;

		pplx::cancellation_token_source _matchmakingCTS;

		GameFinderStatus _currentState = GameFinderStatus::Idle;
		Serializer _serializer;

		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "GameFinder";
	};
}