#pragma once
#include "stdafx.h"

namespace Stormancer
{
	// Different available reliability levels when sending a packet.
	enum PacketReliability
	{
		// The packet may be lost, or arrive out of order. There are no guarantees whatsoever.
		UNRELIABLE = 0,
		// The packets arrive in order, but may be lost. If a packet arrives out of order, it is discarded.
		// The last packet may also never arrive.
		UNRELIABLE_SEQUENCED = 1,
		// The packets always reach destination, but may do so out of order.
		RELIABLE = 2,
		// The packets always reach destination and in order.
		RELIABLE_ORDERED = 3,
		// The packets arrive at destination in order. If a packet arrive out of order, it is ignored.
		// That mean that packets may disappear, but the last one always reach destination.
		RELIABLE_SEQUENCED = 4
	}
};
