#pragma once

#include "cpprest/json.h"

#include "stormancer/RPC/service.h"
#include "LeaderboardModels.h"

namespace Stormancer
{
	class LeaderboardService
	{
	public:

#pragma region public_methods

		LeaderboardService(std::shared_ptr<Scene> scene);

		//Query a leaderboard
		pplx::task<LeaderboardResult> query(LeaderboardQuery query);

		//Query a leaderboard using a cursor obtained from a LeaderboardResult (result.next or result.previous)
		pplx::task<LeaderboardResult> query(const std::string& cursor);

#pragma endregion

	private:

#pragma region

		std::shared_ptr<Scene> _scene = nullptr;
		std::shared_ptr<RpcService> _rpcService;

#pragma endregion
	};
}
