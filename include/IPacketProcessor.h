#pragma once
#include "headers.h"
#include "PacketProcessorConfig.h"

namespace Stormancer
{
	/// Interface describing a packet processor.
	class IPacketProcessor
	{
	public:
		IPacketProcessor();
		virtual ~IPacketProcessor();

	public:
		virtual void registerProcessor(PacketProcessorConfig& config) = 0;
	};
};
