#pragma once
#include "stormancer/stormancer.h"

namespace Stormancer
{
	struct PlayerProfileDto
	{
		std::string Id;
		std::string PlayerId;
		std::string Pseudo;
		std::string SteamId;
		MSGPACK_DEFINE(Id, PlayerId, Pseudo, SteamId)
	};
}