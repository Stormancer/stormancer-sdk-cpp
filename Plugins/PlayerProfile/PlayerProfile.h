#pragma once
#include <string>
#include <msgpack_define.h>

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
