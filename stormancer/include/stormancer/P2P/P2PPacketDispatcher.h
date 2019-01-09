#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPacketProcessor.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	class P2PTunnels;

	class P2PPacketDispatcher :public IPacketProcessor
	{
	public:

		P2PPacketDispatcher(std::shared_ptr<P2PTunnels> tunnels, std::shared_ptr<IConnectionManager> connections, std::shared_ptr<ILogger> logger);
		void registerProcessor(PacketProcessorConfig& config);

	private:
		
		std::shared_ptr<P2PTunnels> _tunnels;
		std::shared_ptr<IConnectionManager> _connections;
		std::shared_ptr<ILogger> _logger;
	};
}
