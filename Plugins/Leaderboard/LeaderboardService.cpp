#include "stormancer/headers.h"
#include "Leaderboard/LeaderboardService.h"
#include "Leaderboard/Leaderboard.h"

namespace Stormancer
{
	LeaderboardService::LeaderboardService(std::shared_ptr<Scene> scene)
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



	Leaderboard::Leaderboard(std::weak_ptr<AuthenticationService> auth) :ClientAPI(auth)
	{

	}

	//Query a leaderboard
	pplx::task<LeaderboardResult> Leaderboard::query(LeaderboardQuery query)
	{
		return getLeaderboardService().then([query](std::shared_ptr<LeaderboardService> service) {return service->query(query); });
	}

	//Query a leaderboard using a cursor obtained from a LeaderboardResult (result.next or result.previous)
	pplx::task<LeaderboardResult> Leaderboard::query(const std::string& cursor)
	{
		return getLeaderboardService().then([cursor](std::shared_ptr<LeaderboardService> service) {return service->query(cursor); });
	}

	pplx::task<std::shared_ptr<LeaderboardService>> Leaderboard::getLeaderboardService()
	{
		return this->getService<LeaderboardService>("stormancer.leaderboard");
	}
}
