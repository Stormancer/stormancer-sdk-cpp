#pragma once

#include "stormancer/headers.h"
#include "stormancer/PacketProcessorConfig.h"

namespace Stormancer
{
	/// Interface describing a packet processor.
	class IPacketProcessor
	{
	public:

		virtual void registerProcessor(PacketProcessorConfig& config) = 0;
	};
};
