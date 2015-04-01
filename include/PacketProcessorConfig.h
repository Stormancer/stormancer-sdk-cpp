#pragma once
#include "headers.h"
#include "Core/Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	using handlerFunction = function < bool(Packet<>) >;
	using processorFunction = function < bool(byte, Packet<>) >;
	using handlerMap = map < byte, handlerFunction >;

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
