#pragma once

#include "stormancer/msgpack_define.h"
#include <string>

namespace Stormancer
{
	struct PlayerDataEntry
	{
		std::string Data;

		int Version = 0;

		MSGPACK_DEFINE(Data, Version);
	};

	struct PlayerDataUpdate
	{
		std::string PlayerId;

		std::string DataKey;

		PlayerDataEntry Data;

		std::string PlayerPlatformId;

		MSGPACK_DEFINE(PlayerId, DataKey, Data, PlayerPlatformId);
	};
}
