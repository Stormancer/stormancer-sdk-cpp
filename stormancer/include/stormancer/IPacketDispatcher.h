#pragma once

#include "stormancer/headers.h"
#include "stormancer/Packet.h"
#include "stormancer/IPacketProcessor.h"

namespace Stormancer
{
	/// Interface describing a message dispatcher.
	class IPacketDispatcher
	{
	public:
		
		/// Dispatches a packet to the system.
		/// \param packet The packet to dispatch.
		virtual void dispatchPacket(Packet_ptr packet) = 0;

		/// Adds a packet processor to the dispatcher.
		/// \param processor An 'IPacketProcessor' object.
		virtual void addProcessor(std::shared_ptr<IPacketProcessor> processor) = 0;
	};
};
