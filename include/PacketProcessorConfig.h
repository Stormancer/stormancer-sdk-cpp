#pragma once
#include "headers.h"
#include "Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	using handlerFunction = function < bool(shared_ptr<Packet<>>) >;
	using processorFunction = function < bool(byte, shared_ptr<Packet<>>) >;

	/// Contains methods to register messages handlers based on message type.
	/// Pass it to the IPacketProcessor::RegisterProcessor method.
	class PacketProcessorConfig
	{
	public:
		PacketProcessorConfig(map<byte, handlerFunction*>& handlers, vector<processorFunction*>& defaultProcessors);
		virtual ~PacketProcessorConfig();

		void addProcessor(byte msgId, handlerFunction* handler);     
		void addCatchAllProcessor(processorFunction* processor);

	private:
		map<byte, handlerFunction*>& _handlers;
		vector<processorFunction*>& _defaultProcessors;
	};
};
