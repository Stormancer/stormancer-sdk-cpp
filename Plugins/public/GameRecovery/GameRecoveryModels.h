#pragma once
#include "stormancer/msgpack_define.h"
#include <string>

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