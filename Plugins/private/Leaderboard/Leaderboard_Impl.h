#pragma once
#include "Core/ClientAPI.h"
#include "Leaderboard/Leaderboard.h"
#include "Leaderboard/LeaderboardModels.h"

namespace Stormancer
{
	class LeaderboardService;

	class Leaderboard_Impl: public ClientAPI<Leaderboard_Impl>, public Leaderboard
	{
	public:

		Leaderboard_Impl(std::weak_ptr<AuthenticationService> auth);

		//Query a leaderboard
		pplx::task<LeaderboardResult> query(LeaderboardQuery query);

		//Query a leaderboard using a cursor obtained from a LeaderboardResult (result.next or result.previous)
		pplx::task<LeaderboardResult> query(const std::string& cursor);

	private:

		pplx::task<std::shared_ptr<LeaderboardService>> getLeaderboardService();
	};
}