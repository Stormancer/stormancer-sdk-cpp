#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	/// Contains methods to register messages handlers based on message type.
	/// Pass it to the IPacketProcessor::RegisterProcessor method.
	class PacketProcessorConfig
	{
	public:

		PacketProcessorConfig(std::unordered_map<byte, HandlerFunction>& handlers, std::vector<ProcessorFunction>& defaultProcessors);

		virtual ~PacketProcessorConfig();

		void addProcessor(byte msgId, HandlerFunction handler);     

		void addCatchAllProcessor(ProcessorFunction processor);

	private:

		std::unordered_map<byte, HandlerFunction>& _handlers;

		std::vector<ProcessorFunction>& _defaultProcessors;
	};
}
