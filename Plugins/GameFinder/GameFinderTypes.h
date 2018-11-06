#pragma once
#include <unordered_map>
namespace Stormancer
{
	enum class Readiness
	{
		Unknown = 0,
		Ready = 1,
		NotReady = 2
	};

	enum class GameFinderStatus
	{
		
		Idle = -1,
		
		Searching = 0,
		CandidateFound = 1,
		WaitingPlayersReady = 2,
		Success = 3,
		Failed = 4,
		Canceled = 5,
		Loading = 6
	};

	struct GameFinderResponse
	{

		std::string connectionToken;
		std::unordered_map<std::string, std::string> optionalParameters;
	};
}