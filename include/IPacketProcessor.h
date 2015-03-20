#pragma once
#include "libs.h"
#include "PacketProcessorConfig.h"

namespace Stormancer
{
	using handlerFunction = function < bool(byte, Packet2) >;
	using processorFunction = function < bool(byte, Packet2) >;
	using handlerMap = map < byte, handlerFunction >;

	class IPacketProcessor
	{
	public:
		IPacketProcessor();
		virtual ~IPacketProcessor();

		virtual void registerProcessor(PacketProcessorConfig config) = 0;
	};
};
