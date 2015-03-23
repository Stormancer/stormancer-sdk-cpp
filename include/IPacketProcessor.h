#pragma once
#include "libs.h"
#include "CustomTypes.h"
#include "PacketProcessorConfig.h"

namespace Stormancer
{
	class IPacketProcessor
	{
	public:
		IPacketProcessor();
		virtual ~IPacketProcessor();

		virtual void registerProcessor(PacketProcessorConfig config) = 0;
	};
};
