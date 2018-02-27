#pragma once

#include "stormancer/headers.h"
#include "stormancer/P2P/P2PEnums.h"

namespace Stormancer
{
	struct P2PConnectionStateChangedArgs
	{
		uint64 peerId;
		P2PConnectionStateChangeType type;
	};
}
