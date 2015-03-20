#pragma once
#include "stdafx.h"
#include "Core/Packet.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	class IPacketDispatcher
	{
	public:
		IPacketDispatcher();
		virtual ~IPacketDispatcher();

		virtual void dispatchPacket(Packet2 packet) = 0;

		virtual void addProcessor(shared_ptr<IPacketProcessor*> processor) = 0;
	};
};
