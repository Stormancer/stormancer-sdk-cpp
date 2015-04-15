#pragma once
#include "headers.h"
#include "Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	class IPacketDispatcher
	{
	public:
		IPacketDispatcher();
		virtual ~IPacketDispatcher();

		virtual void dispatchPacket(Packet<>* packet) = 0;

		virtual void addProcessor(IPacketProcessor* processor) = 0;
	};
};
