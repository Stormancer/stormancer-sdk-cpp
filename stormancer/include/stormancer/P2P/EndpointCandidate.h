#pragma once

#include "stormancer/headers.h"
#include "stormancer/P2P/P2PEnums.h"

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
