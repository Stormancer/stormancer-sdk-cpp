#pragma once
#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/Packet.h"
#include <unordered_map>
#include <string>
#include <map>
#include <vector>

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

		Packetisp_ptr packet;

		template<typename TData>
		TData readData()
		{
			Serializer serializer;
			return serializer.deserializeOne<TData>(packet->stream);
		}

		template<typename... TData>
		void readData(TData&... tData)
		{
			Serializer serializer;
			return serializer.deserialize<TData...>(packet->stream, tData...);
		}
	};
	
	struct GameFinderStatusChangedEvent
	{
		GameFinderStatus status;
		std::string gameFinder;
	};
	
	struct GameFoundEvent
	{
		std::string gameFinder;
		GameFinderResponse data;
	};

	struct FindGameFailedEvent
	{
		std::string reason;
		std::string gameFinder;
	};

	struct GameFinderRequest
	{
		std::map<std::string, std::string> profileIds;
		std::string MapName;

		MSGPACK_DEFINE(profileIds, MapName);
	};

	struct PlayerDto
	{
		uint64 steamId;
		std::string playerId;
		std::string steamName;

		MSGPACK_DEFINE(steamId, playerId, steamName);
	};

	struct TeamDto
	{
		std::vector<PlayerDto> team;

		MSGPACK_DEFINE(team)
	};

	struct GameFinderResponseDto
	{
		std::string connectionToken;

		MSGPACK_DEFINE(connectionToken);
	};

	struct ReadyVerificationRequest
	{
		std::map<std::string, Readiness> members;
		std::string gameId;
		int32 timeout;
		int32 membersCountReady;
		int32 membersCountTotal;
	};
}
