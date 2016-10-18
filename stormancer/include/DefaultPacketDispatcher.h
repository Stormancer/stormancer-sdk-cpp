#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	/// Dispatch the packets to the handlers based on their id.
	class DefaultPacketDispatcher : public IPacketDispatcher
	{
	public:
		DefaultPacketDispatcher(bool asyncDispatch = true);
		virtual ~DefaultPacketDispatcher();

		void dispatchPacket(Packet_ptr packet);
		void addProcessor(std::shared_ptr<IPacketProcessor> processor);

	private:
		void dispatchImpl(Packet_ptr packet);

	private:
		std::map<byte, handlerFunction*> _handlers;
		std::vector<processorFunction*> _defaultProcessors;
		bool _asyncDispatch = true;
	};
};
