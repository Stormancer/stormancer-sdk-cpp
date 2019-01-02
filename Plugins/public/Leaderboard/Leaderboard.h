#pragma once
#include "Leaderboard/LeaderboardModels.h"
#include "stormancer/headers.h"

namespace Stormancer
{
	class Leaderboard 
	{
	public:
		//Query a leaderboard
		virtual pplx::task<LeaderboardResult> query(LeaderboardQuery query) = 0;

		//Query a leaderboard using a cursor obtained from a LeaderboardResult (result.next or result.previous)
		virtual pplx::task<LeaderboardResult> query(const std::string& cursor) = 0;
	};
}