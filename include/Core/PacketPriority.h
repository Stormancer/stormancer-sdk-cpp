#pragma once
#include "libs.h"

namespace Stormancer
{
	// Available packet priorities
	// If packets are in competition for expedition, priority levels work as follow:
	// 2 HIGH_PRIORITY packets are sent for 1 MEDIUM_PRIORITY packet.
	// 2 MEDIUM_PRIORITY packets are sent for 1 LOW_PRIORITY packet.
	enum PacketPriority
	{
		// The packet is sent immediately without aggregation.
		IMMEDIATE_PRIORITY = 0,
		// The packet is sent at high priority level.
		HIGH_PRIORITY = 1,
		// The packet is sent at medium priority level.
		MEDIUM_PRIORITY = 2,
		// The packet is sent at low priority level.
		LOW_PRIORITY = 3
	};
};
