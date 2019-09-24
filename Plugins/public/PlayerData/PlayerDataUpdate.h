#pragma once

#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"
#include <string>
#include <vector>

namespace Stormancer
{
	struct PlayerDataEntry
	{
		// msgpack has predefined adaptors for vector<char> and vector<unsigned char>, which treats them as "bin" format.
		std::vector<byte> Data;

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
