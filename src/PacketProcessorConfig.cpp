#include "PacketProcessorConfig.h"
#include "Helpers.h"

namespace Stormancer
{
	PacketProcessorConfig::PacketProcessorConfig(handlerMap handlers, list<processorFunction> defaultProcessors)
		: _handlers(handlers),
		_defaultProcessors(defaultProcessors)
	{
	}

	PacketProcessorConfig::~PacketProcessorConfig()
	{
	}

	void PacketProcessorConfig::addProcessor(byte msgId, handlerFunction handler)
	{
		if (Helpers::mapContains(_handlers, msgId))
		{
			stringstream sstm;
			sstm << "An handler is already registered for id " << (int)msgId;
			throw new exception(sstm.str().c_str());
		}
		_handlers[msgId] = handler;
	}

	void PacketProcessorConfig::addCatchAllProcessor(handlerFunction handler)
	{
		_defaultProcessors.push_back(handler);
	}
};
