#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	/// Dispatch the packets to the handlers based on their id.
	class DefaultPacketDispatcher : public IPacketDispatcher
	{
	public:
		DefaultPacketDispatcher();
		virtual ~DefaultPacketDispatcher();

		void dispatchPacket(std::shared_ptr<Packet<>> packet);
		void addProcessor(IPacketProcessor* processor);

	private:
		std::map<byte, handlerFunction*> _handlers;
		std::vector<processorFunction*> _defaultProcessors;
	};
};
