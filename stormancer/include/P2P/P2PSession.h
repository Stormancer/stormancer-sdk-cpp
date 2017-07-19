#pragma once

#include "headers.h"
#include "P2P/P2PEnums.h"

namespace Stormancer
{
	class P2PSession
	{
	public:
		std::vector<char> sessionId;
		uint64 remotePeer;
		std::string sceneId;

		P2PSessionState Status;

		MSGPACK_DEFINE(sessionId, remotePeer, sceneId)
	};
}
