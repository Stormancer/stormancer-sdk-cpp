#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/ITransport.h"
#include "stormancer/Serializer.h"
#include "stormancer/P2P/P2PTunnel.h"
#include "stormancer/P2P/P2PSessions.h"

namespace Stormancer
{
	class P2PTunnels;

	class P2PService : public std::enable_shared_from_this<P2PService>
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

		pplx::task<std::shared_ptr<P2PTunnel>> openTunnel(uint64 connectionId, const std::string& id, pplx::cancellation_token ct = pplx::cancellation_token::none());

		pplx::task<std::shared_ptr<IConnection>> openP2PConnection(std::shared_ptr<IConnection> server, const std::string& p2pToken, pplx::cancellation_token ct = pplx::cancellation_token::none());

	private:

		std::shared_ptr<RequestProcessor> _sysCall;
		std::shared_ptr<IConnectionManager> _connections;
		std::shared_ptr<ITransport> _transport;
		std::shared_ptr<Serializer> _serializer;
		std::shared_ptr<P2PTunnels> _tunnels;
		std::shared_ptr<P2PSessions> _p2pSessions;
	};
}
