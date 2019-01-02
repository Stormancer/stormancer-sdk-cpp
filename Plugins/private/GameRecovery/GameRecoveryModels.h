#pragma once
#include "stormancer/headers.h"

namespace Stormancer
{
	struct RecoverableGameDto
	{
		std::string gameId;
		std::string userData;
		MSGPACK_DEFINE(gameId, userData)
	};

	struct RecoverableGame
	{
		std::string gameId;
		std::string userData;
	};
}