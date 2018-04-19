#pragma once

#include "stormancer/headers.h"
#include "stormancer/P2P/P2PEnums.h"

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
