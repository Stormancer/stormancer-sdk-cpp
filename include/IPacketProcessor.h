#pragma once
#include "headers.h"
#include "PacketProcessorConfig.h"

namespace Stormancer
{
	class IPacketProcessor
	{
	public:
		IPacketProcessor();
		virtual ~IPacketProcessor();

		virtual void registerProcessor(PacketProcessorConfig* config) = 0;
	};
};
