#pragma once
#include "headers.h"
#include "Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	/// Interface describing a message dispatcher.
	class IPacketDispatcher
	{
	public:
		
		/// Constructor.
		IPacketDispatcher();
		
		/// Destructor.
		virtual ~IPacketDispatcher();

		/*! Dispatches a packet to the system.
		\param packet The packet to dispatch.
		*/
		virtual void dispatchPacket(shared_ptr<Packet<>> packet) = 0;

		/*! Adds a packet processor to the dispatcher.
		\param processor An 'IPacketProcessor' object.
		*/
		virtual void addProcessor(IPacketProcessor* processor) = 0;
	};
};
