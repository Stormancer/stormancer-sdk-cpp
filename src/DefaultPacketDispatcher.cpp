#include "stormancer.h"

namespace Stormancer
{
	DefaultPacketDispatcher::DefaultPacketDispatcher()
	{
	}

	DefaultPacketDispatcher::~DefaultPacketDispatcher()
	{
	}

	void DefaultPacketDispatcher::dispatchPacket(Packet<>* packet)
	{
		// TODO
		/*async(launch::async, [this](Packet<>* packet2) {
			bool processed = false;
			int count = 0;
			byte msgType = 0;
			while (!processed && count < 40) // Max 40 layers
			{
				*packet2->stream >> msgType;
				if (Helpers::mapContains(_handlers, msgType))
				{
					handlerFunction handler = _handlers[msgType];
					count++;
				}
				else
				{
					break;
				}
			}
			for each (auto processor in _defaultProcessors)
			{
				if (processor(msgType, packet2))
				{
					processed = true;
					break;
				}
			}
			if (!processed)
			{
				stringstream sstm;
				sstm << "Couldn't process message. msgId: " << msgType;
				throw new exception(sstm.str().c_str());
			}
		});*/
	}

	void DefaultPacketDispatcher::addProcessor(IPacketProcessor* processor)
	{
		processor->registerProcessor(new PacketProcessorConfig(_handlers, _defaultProcessors));
	}
};
