#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/IPacketDispatcher.h"
#include "stormancer/IPacketProcessor.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	/// Dispatch the packets to the handlers based on their id.
	class DefaultPacketDispatcher : public IPacketDispatcher
	{
	public:

		DefaultPacketDispatcher(ILogger_ptr logger, bool asyncDispatch = true);

		virtual ~DefaultPacketDispatcher();

		void dispatchPacket(Packet_ptr packet);

		void addProcessor(std::shared_ptr<IPacketProcessor> processor);

	private:

		void dispatchImpl(Packet_ptr packet);

		std::unordered_map<byte, HandlerFunction> _handlers;

		std::vector<ProcessorFunction> _defaultProcessors;

		bool _asyncDispatch = true;

		int _maxLayerCount = 0;

		ILogger_ptr _logger;
	};
}
