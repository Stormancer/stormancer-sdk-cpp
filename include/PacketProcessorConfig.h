#pragma once
#include "libs.h"
#include "Core/Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	using handlerFunction = function < bool(Packet2) >;
	using processorFunction = function < bool(byte, Packet2) >;
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
