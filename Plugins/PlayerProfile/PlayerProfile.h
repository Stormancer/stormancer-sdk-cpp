#pragma once
#include <headers.h>

namespace Stormancer
{
	class PlayerProfile
	{
	public:
		std::string playerId;
		std::string name;
		//...

		MSGPACK_DEFINE(playerId, name);
	};
}
