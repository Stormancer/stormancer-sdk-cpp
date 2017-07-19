#pragma once

#include "headers.h"
#include "IPacketProcessor.h"
#include "IConnectionManager.h"

namespace Stormancer
{
	class P2PTunnels;

	class P2PPacketDispatcher :public IPacketProcessor
	{
	public:

		P2PPacketDispatcher(std::shared_ptr<P2PTunnels> tunnels, std::shared_ptr<IConnectionManager> connections);
		void registerProcessor(PacketProcessorConfig& config);

	private:
		
		std::shared_ptr<P2PTunnels> _tunnels;
		std::shared_ptr<IConnectionManager> _connections;
	};
}
