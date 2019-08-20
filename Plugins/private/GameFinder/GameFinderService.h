#pragma once
#include <memory>

#include "GameFinder/GameFinderModels.h"
#include "stormancer/Event.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class RpcService;
	class ILogger;

	namespace Internal
	{
		struct ReadyVerificationRequest
		{
			std::map<std::string, int32> members;
			std::string gameId;
			int32 timeout;

			MSGPACK_DEFINE(members, gameId, timeout);

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
		pplx::task<void> findGame(const std::string &provider, const StreamWriter& streamWriter);
		void resolve(bool acceptGame);
		// If cancel() is called very shortly after findGame(), there might be a race condition.
		// It can be prevented by waiting for the first GameFinderStatusUpdated event received after calling findGame() before calling cancel().
		void cancel();
		// This should only be called by GameFinderPlugin.
		void onSceneDisconnecting();

		template<typename... TData>
		pplx::task<void> findGame(const std::string &provider, const TData&... tData)
		{
			return findGameInternal(provider, tData...);
		}

		Event<GameFinderStatus> GameFinderStatusUpdated;
		Event<GameFinderResponse> GameFound;
		Event<std::string> FindGameRequestFailed;

	private:

		pplx::task<void> findGameInternal(const std::string& provider, const StreamWriter& streamWriter);

		template<typename... TData>
		pplx::task<void> findGameInternal(const std::string& provider, TData... tData)
		{
			StreamWriter streamWriter = [tData...](obytestream* stream)
			{
				Serializer serializer;
				serializer.serialize(stream, tData...);
			};
			return findGameInternal(provider, streamWriter);
		}

		std::weak_ptr<Scene> _scene;
		std::weak_ptr<RpcService> _rpcService;

		pplx::cancellation_token_source _gameFinderCTS;

		GameFinderStatus _currentState = GameFinderStatus::Idle;
		Serializer _serializer;

		std::shared_ptr<Stormancer::ILogger> _logger;
		std::string _logCategory = "GameFinder";
	};
}
