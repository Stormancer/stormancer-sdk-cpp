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
		pplx::create_task([this, packet]() {
			bool processed = false;
			int count = 0;
			byte msgType = 0;
			while (!processed && count < 40) // Max 40 layers
			{
				*(packet->stream) >> msgType;
				if (Helpers::mapContains(this->_handlers, msgType))
				{
					handlerFunction* handler = this->_handlers[msgType];
					processed = (*handler)(packet);
					count++;
				}
				else
				{
					break;
				}
			}
			for (auto processor : this->_defaultProcessors)
			{
				if ((*processor)(msgType, packet))
				{
					processed = true;
					break;
				}
			}
			if (!processed)
			{
				throw exception(string(Helpers::StringFormat(L"Couldn't process message. msgId: ", msgType)).c_str());
			}
		});
	}

	void DefaultPacketDispatcher::addProcessor(IPacketProcessor* processor)
	{
		processor->registerProcessor(new PacketProcessorConfig(_handlers, _defaultProcessors));
	}
};
