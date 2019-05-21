#pragma once

#include "stormancer/BuildConfig.h"

namespace Stormancer
{
	class OpenRelayParameters
	{
	public:

		std::vector<byte> sessionId;
		uint64 remotePeerId;
		std::string remotePeerAddress;

		MSGPACK_DEFINE(sessionId, remotePeerId, remotePeerAddress);
	};
}
