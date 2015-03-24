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

		void dispatchPacket(Packet2 packet);
		void addProcessor(shared_ptr<IPacketProcessor*> processor);

	private:
		map<byte, handlerFunction> _handlers;
		list<processorFunction> _defaultProcessors;
	};
};
