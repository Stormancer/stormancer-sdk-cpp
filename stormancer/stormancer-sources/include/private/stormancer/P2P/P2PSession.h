#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/P2P/P2PEnums.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include <vector>
#include <string>

namespace Stormancer
{
	struct P2PSession
	{
		std::vector<char> sessionId;
		uint64 remotePeer = 0;
		std::string sceneId;
		P2PSessionState Status = P2PSessionState::Unknown;

		MSGPACK_DEFINE(sessionId, remotePeer, sceneId)
	};
}
