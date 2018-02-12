#pragma once

#include "headers.h"
#include "IConnectionManager.h"
#include "RequestProcessor.h"
#include "ITransport.h"
#include "Serializer.h"
#include "P2P/P2PTunnel.h"

namespace Stormancer
{
	class P2PTunnels;

	class P2PService
	{
	public:
		
		P2PService(
			std::shared_ptr<IConnectionManager> connections,
			std::shared_ptr<RequestProcessor> sysClient,
			std::shared_ptr<ITransport> transport,
			std::shared_ptr<Serializer> serializer,
			std::shared_ptr<P2PTunnels> tunnels
		);

		std::shared_ptr<P2PTunnel> registerP2PServer(const std::string& p2pServerId);

		pplx::task<std::shared_ptr<P2PTunnel>> openTunnel(uint64 connectionId, const std::string& id);

		pplx::task<std::shared_ptr<IConnection>> openP2PConnection(const std::string& token);

	private:

		std::shared_ptr<RequestProcessor> _sysCall;
		std::shared_ptr<IConnectionManager> _connections;
		std::shared_ptr<ITransport> _transport;
		std::shared_ptr<Serializer> _serializer;
		std::shared_ptr<P2PTunnels> _tunnels;
	};
}