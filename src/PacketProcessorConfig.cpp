#include "stormancer.h"

namespace Stormancer
{
	PacketProcessorConfig::PacketProcessorConfig(map<byte, handlerFunction>& handlers, vector<processorFunction>& defaultProcessors)
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
			throw string(Helpers::StringFormat(L"An handler is already registered for id {0}.", msgId));
		}
		_handlers[msgId] = handler;
	}

	void PacketProcessorConfig::addCatchAllProcessor(processorFunction processor)
	{
		_defaultProcessors.push_back(processor);
	}
};
