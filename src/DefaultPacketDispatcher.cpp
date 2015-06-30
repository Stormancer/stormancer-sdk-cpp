#include "stormancer.h"

namespace Stormancer
{
	DefaultPacketDispatcher::DefaultPacketDispatcher()
	{
	}

	DefaultPacketDispatcher::~DefaultPacketDispatcher()
	{
		for (auto pair : _handlers)
		{
			delete pair.second;
		}
		_handlers.clear();

		for (auto processor : _defaultProcessors)
		{
			delete processor;
		}
		_defaultProcessors.clear();
	}

	void DefaultPacketDispatcher::dispatchPacket(std::shared_ptr<Packet<>> packet)
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
				throw std::runtime_error(Helpers::stringFormat("Couldn't process message. msgId: ", msgType));
			}
		});
	}

	void DefaultPacketDispatcher::addProcessor(IPacketProcessor* processor)
	{
		PacketProcessorConfig config(_handlers, _defaultProcessors);
		processor->registerProcessor(config);
	}
};
