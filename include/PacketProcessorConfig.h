#pragma once
#include "headers.h"
#include "Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	using handlerFunction = function < bool(Packet<>*) >;
	using processorFunction = function < bool(byte, Packet<>*) >;

	class PacketProcessorConfig
	{
	public:
		PacketProcessorConfig(map<byte, handlerFunction>& handlers, vector<processorFunction>& defaultProcessors);
		virtual ~PacketProcessorConfig();

		void addProcessor(byte msgId, handlerFunction handler);
		void addCatchAllProcessor(processorFunction processor);

	private:
		map<byte, handlerFunction> _handlers;
		vector<processorFunction> _defaultProcessors;
	};
};
