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

		void dispatchPacket(Packet packet) = 0;

		void addProcessor(IPacketProcessor processor) = 0;
	};
};
