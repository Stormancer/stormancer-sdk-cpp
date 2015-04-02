#pragma once
#include "headers.h"
#include "Core/Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	class IPacketDispatcher
	{
	public:
		IPacketDispatcher();
		virtual ~IPacketDispatcher();

		virtual void dispatchPacket(Packet<> packet) = 0;

		virtual void addProcessor(shared_ptr<IPacketProcessor> processor) = 0;
	};
};
