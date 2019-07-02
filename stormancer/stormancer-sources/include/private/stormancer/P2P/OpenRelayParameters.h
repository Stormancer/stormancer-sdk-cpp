#pragma once

#include "stormancer/BuildConfig.h"

namespace Stormancer
{
	class OpenRelayParameters
	{
	public:

		std::vector<byte> p2pSessionId;
		uint64 remotePeerId;
		std::string remotePeerAddress;

		MSGPACK_DEFINE(p2pSessionId, remotePeerId, remotePeerAddress);
	};
}
