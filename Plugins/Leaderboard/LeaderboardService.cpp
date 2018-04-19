#include "stormancer/headers.h"
#include "LeaderboardService.h"

namespace Stormancer
{
	LeaderboardService::LeaderboardService(Scene* scene)
	{
		_scene = scene;
		if (scene)
		{
			_rpcService = scene->dependencyResolver()->resolve<RpcService>();
		}
	}

	pplx::task<LeaderboardResult> LeaderboardService::query(LeaderboardQuery query)
	{
		return _rpcService->rpc<LeaderboardResult, LeaderboardQuery>("leaderboard.query", query);
	}

	pplx::task<LeaderboardResult> LeaderboardService::query(const std::string& cursor)
	{
		return _rpcService->rpc<LeaderboardResult, std::string>("leaderboard.cursor", cursor);
	}
}
