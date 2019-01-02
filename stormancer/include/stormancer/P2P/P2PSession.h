#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/P2P/P2PEnums.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include <vector>
#include <string>

namespace Stormancer
{
	class P2PSession
	{
	public:
		P2PSession() :
			sessionId(),
			remotePeer(0),
			sceneId(),
			Status(P2PSessionState::Unknown)
		{

		}
		std::vector<char> sessionId;
		uint64 remotePeer;
		std::string sceneId;

		P2PSessionState Status;

		MSGPACK_DEFINE(sessionId, remotePeer, sceneId)
	};
}
