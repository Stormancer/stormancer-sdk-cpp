#include "stormancer.h"

namespace Stormancer
{
	DefaultPacketDispatcher::DefaultPacketDispatcher(bool asyncDispatch)
		: _asyncDispatch(asyncDispatch)
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

	void DefaultPacketDispatcher::dispatchPacket(Packet_ptr packet)
	{
		if (_asyncDispatch)
		{
			pplx::create_task([this, packet]() {
				this->dispatchImpl(packet);
			}).then([](pplx::task<void> t) {
				try
				{
					t.wait();
				}
				catch (const std::exception& e)
				{
					int a = 1337;
				}
			});
		}
		else
		{
			dispatchImpl(packet);
		}
	}

	void DefaultPacketDispatcher::dispatchImpl(Packet_ptr packet)
	{
		bool processed = false;
		int count = 0;
		byte msgType = 0;
		while (!processed && count < 40) // Max 40 layers
		{
			*(packet->stream) >> msgType;
			if (mapContains(this->_handlers, msgType))
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
			throw std::runtime_error(std::string("Couldn't process message. msgId: ") + std::to_string(msgType));
		}
	}

	void DefaultPacketDispatcher::addProcessor(IPacketProcessor* processor)
	{
		PacketProcessorConfig config(_handlers, _defaultProcessors);
		processor->registerProcessor(config);
	}
};
