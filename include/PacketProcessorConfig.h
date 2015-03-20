#pragma once
#include "stdafx.h"
#include "Core/Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	class PacketProcessorConfig
	{
	public:
		PacketProcessorConfig(handlerMap handlers, list<processorFunction> defaultProcessors);
		virtual ~PacketProcessorConfig();

		void addProcessor(byte msgId, handlerFunction handler);
		void addCatchAllProcessor(handlerFunction handler);

	private:
		handlerMap _handlers;
		list<processorFunction> _defaultProcessors;
	};
};
