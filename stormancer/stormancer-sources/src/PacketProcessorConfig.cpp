#include "stormancer/stdafx.h"
#include "stormancer/PacketProcessorConfig.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	PacketProcessorConfig::PacketProcessorConfig(std::unordered_map<byte, HandlerFunction>& handlers, std::vector<ProcessorFunction>& defaultProcessors)
		: _handlers(handlers)
		, _defaultProcessors(defaultProcessors)
	{
	}

	PacketProcessorConfig::~PacketProcessorConfig()
	{
	}

	void PacketProcessorConfig::addProcessor(byte msgId, HandlerFunction handler)
	{
		if (mapContains(_handlers, msgId))
		{
			throw std::invalid_argument(std::string("An handler is already registered for id ") + std::to_string(msgId) + ".");
		}
		_handlers[msgId] = handler;
	}

	void PacketProcessorConfig::addCatchAllProcessor(ProcessorFunction processor)
	{
		_defaultProcessors.push_back(processor);
	}
}
