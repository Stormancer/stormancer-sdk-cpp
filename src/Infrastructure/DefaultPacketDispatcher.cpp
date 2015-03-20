#include "libs.h"
#include "Infrastructure/DefaultPacketDispatcher.h"
#include "Helpers.h"

namespace Stormancer
{
	DefaultPacketDispatcher::DefaultPacketDispatcher()
	{
	}

	DefaultPacketDispatcher::~DefaultPacketDispatcher()
	{
	}

	void DefaultPacketDispatcher::dispatchPacket(Packet2 packet)
	{
		// TODO
		async(launch::async, [&](Packet2 packet) {
			bool processed = false;
			int count = 0;
			byte msgType = 0;
			while (!processed && count < 40) // Max 40 layers
			{
				msgType = packet.stream.sbumpc();
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
				if (processor(msgType, packet))
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
		});
	}

	void DefaultPacketDispatcher::addProcessor(shared_ptr<IPacketProcessor*> processor)
	{
		(*processor)->registerProcessor(PacketProcessorConfig(_handlers, _defaultProcessors));
	}
};
