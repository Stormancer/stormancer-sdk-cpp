#include "stormancer.h"

namespace Stormancer
{
	PacketProcessorConfig::PacketProcessorConfig(std::map<byte, handlerFunction*>& handlers, std::vector<processorFunction*>& defaultProcessors)
		: _handlers(handlers),
		_defaultProcessors(defaultProcessors)
	{
	}

	PacketProcessorConfig::~PacketProcessorConfig()
	{
	}

	void PacketProcessorConfig::addProcessor(byte msgId, handlerFunction* handler)
	{
		if (mapContains(_handlers, msgId))
		{
			throw std::invalid_argument(stringFormat("An handler is already registered for id ", msgId, ".").c_str());
		}
		_handlers[msgId] = handler;
	}

	void PacketProcessorConfig::addCatchAllProcessor(processorFunction* processor)
	{
		_defaultProcessors.push_back(processor);
	}
};
