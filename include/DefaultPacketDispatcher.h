#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	class DefaultPacketDispatcher : public IPacketDispatcher
	{
	public:
		DefaultPacketDispatcher();
		virtual ~DefaultPacketDispatcher();

		void dispatchPacket(shared_ptr<Packet<>> packet);
		void addProcessor(IPacketProcessor* processor);

	private:
		map<byte, handlerFunction*> _handlers;
		vector<processorFunction*> _defaultProcessors;
	};
};
