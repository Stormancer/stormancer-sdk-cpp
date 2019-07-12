#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/msgpack_define.h"
#include <string>

namespace Stormancer
{
	struct P2PDisconnectFromSceneMessage
	{
		/// Scene id
		std::string sceneId;

		/// Disconnection reason
		std::string reason;

		MSGPACK_DEFINE(sceneId, reason);
	};
}
