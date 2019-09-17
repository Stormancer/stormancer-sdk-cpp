#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/P2P/P2PEnums.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/Subscription.h"
#include <vector>
#include <string>

namespace Stormancer
{
	struct P2PSession
	{
		std::vector<byte> SessionId;
		uint64 RemotePeerId = 0;
		std::string SceneId;
		std::string RemoteSessionId;
		std::string OwningSessionId;
		P2PSessionState Status = P2PSessionState::Unknown;
		Subscription onConnectionCloseSubscription;

		MSGPACK_DEFINE_MAP(SessionId, RemotePeerId, SceneId, RemoteSessionId, OwningSessionId, Status)
	};

	struct P2PSessionResult
	{
		std::vector<byte> SessionId;
		uint64 remotePeerId;
		std::string SceneId;
		std::string RemoteSessionId;

		MSGPACK_DEFINE(SessionId, remotePeerId, SceneId, RemoteSessionId)
	};
}
