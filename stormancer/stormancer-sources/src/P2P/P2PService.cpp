#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PService.h"
#include "stormancer/P2P/P2PSession.h"
#include "stormancer/P2P/P2PSessions.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/Exceptions.h"
#include "cpprest/asyncrt_utils.h"
#include "stormancer/Utilities/PointerUtilities.h"

namespace Stormancer
{
	P2PService::P2PService(std::shared_ptr<IConnectionManager> connections, std::shared_ptr<RequestProcessor> sysClient, std::shared_ptr<ITransport> transport, std::shared_ptr<Serializer> serializer, std::shared_ptr<P2PTunnels> tunnels)
	{
		_connections = connections;
		_sysCall = sysClient;
		_transport = transport;
		_serializer = serializer;
		_tunnels = tunnels;
	}

	std::shared_ptr<P2PTunnel> P2PService::registerP2PServer(const std::string& p2pServerId)
	{
		return _tunnels->createServer(p2pServerId, _tunnels);
	}

	pplx::task<std::shared_ptr<P2PTunnel>> P2PService::openTunnel(std::string sessionId, const std::string& id, pplx::cancellation_token ct)
	{
		return _tunnels->openTunnel(sessionId, id, ct);
	}

	pplx::task<std::shared_ptr<IConnection>> P2PService::openP2PConnection(std::shared_ptr<IConnection> server, const std::string& p2pToken, pplx::cancellation_token ct)
	{
		if (!server)
		{
			return pplx::task_from_exception<std::shared_ptr<IConnection>>(std::runtime_error("Cannot establish P2P connection. The client must be connected to the server"));
		}

		auto wP2pService = STORM_WEAK_FROM_THIS();
		return _sysCall->sendSystemRequest<P2PSessionResult>(server.get(), (byte)SystemRequestIDTypes::ID_P2P_CREATE_SESSION, p2pToken, ct)
			.then([wP2pService](P2PSessionResult sessionResult)
		{
			auto p2pService = wP2pService.lock();
			if (!p2pService)
			{
				throw PointerDeletedException("P2P service deleted");
			}
			auto connection = p2pService->_connections->getConnection(sessionResult.RemoteSessionId);
			if (!connection)
			{
				throw std::runtime_error(("Failed to get P2P connection for peer " + sessionResult.RemoteSessionId).c_str());
			}
			else
			{
				return connection;
			}
		}, ct);
	}
}
