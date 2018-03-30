#pragma once

#include "stormancer/headers.h"
#include "stormancer/IRequestModule.h"
#include "stormancer/ITransport.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/P2P/P2PSessions.h"
#include "stormancer/Serializer.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Configuration.h"

namespace Stormancer
{
	class P2PTunnels;

	class P2PRequestModule : public IRequestModule
	{
	public:

		P2PRequestModule(
			std::shared_ptr<ITransport> transport,
			std::shared_ptr<IConnectionManager> connections,
			std::shared_ptr<P2PSessions> sessions,
			std::shared_ptr<Serializer> serializer,
			std::shared_ptr<P2PTunnels> tunnels,
			std::shared_ptr<ILogger> logger, 
			std::shared_ptr<Configuration> config);

		void registerModule(RequestModuleBuilder* builder);

	private:

		std::shared_ptr<ITransport> _transport;
		std::shared_ptr<IConnectionManager> _connections;
		std::shared_ptr<P2PSessions> _sessions;
		std::shared_ptr<Serializer> _serializer;
		std::shared_ptr<P2PTunnels> _tunnels;
		std::shared_ptr<ILogger> _logger;
		std::shared_ptr<Configuration> _config;
	};
}
