#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/P2P/P2PEnums.h"
#include "stormancer/msgpack_define.h"
#include <string>

namespace Stormancer
{
	class EndpointCandidate
	{
	public:
		std::string address;
		int _type;
		EndpointCandidateType type();

		MSGPACK_DEFINE(address, _type)
	};
}
