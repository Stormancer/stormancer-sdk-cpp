#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/stormancerTypes.h"
#include "stormancer/P2P/P2PEnums.h"

namespace Stormancer
{
	struct P2PConnectionStateChangedArgs
	{
		uint64 peerId;
		P2PConnectionStateChangeType type;
	};
}
