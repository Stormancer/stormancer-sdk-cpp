#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PService.h"
#include "stormancer/P2P/P2PSession.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/exceptions.h"

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

	pplx::task<std::shared_ptr<P2PTunnel>> P2PService::openTunnel(uint64 connectionId, const std::string& id, pplx::cancellation_token ct)
	{
		return _tunnels->openTunnel(connectionId, id, ct);
	}

	pplx::task<std::shared_ptr<IConnection>> P2PService::openP2PConnection(std::shared_ptr<IConnection> server, const std::string& p2pToken, pplx::cancellation_token ct)
	{
		
		if (!server)
		{
			return pplx::task_from_exception<std::shared_ptr<IConnection>>(std::runtime_error("Cannot establish P2P connection. The client must be connected to the server"));
		}

		std::weak_ptr<P2PService> weakSelf = this->shared_from_this();
		return _sysCall->sendSystemRequest<P2PSession>(server.get(), (byte)SystemRequestIDTypes::ID_P2P_CREATE_SESSION, p2pToken, ct)
			.then([weakSelf](P2PSession session)
		{
			auto self = weakSelf.lock();
			if (!self)
			{
				throw PointerDeletedException();
			}

			auto c = self->_connections->getConnection(session.remotePeer);
			if (!c)
			{
				throw std::runtime_error(("Failed to get P2P connection for peer " + std::to_string(session.remotePeer)).c_str());
			}
			else
			{
				return c;
			}
		}, ct);
	}
};
