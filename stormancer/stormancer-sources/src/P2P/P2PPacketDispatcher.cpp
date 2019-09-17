#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PPacketDispatcher.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include <cpprest/asyncrt_utils.h>
namespace Stormancer
{
	P2PPacketDispatcher::P2PPacketDispatcher(std::shared_ptr<P2PTunnels> tunnels, std::shared_ptr<IConnectionManager> connections, std::shared_ptr<ILogger> logger, std::weak_ptr<Serializer> serializer)
		: _tunnels(tunnels)
		, _connections(connections)
		, _logger(logger)
		, _serializer(serializer)
	{
	}

	void P2PPacketDispatcher::registerProcessor(PacketProcessorConfig& config)
	{
		auto wSerializer = _serializer;
		std::weak_ptr<IConnectionManager> wConnections = _connections;

		config.addProcessor((byte)MessageIDTypes::ID_P2P_RELAY, [wSerializer, wConnections](Packet_ptr p)
			{
				std::vector<byte> peerId;
				auto serializer = wSerializer.lock();
				if (!serializer)
				{
					assert(false);
					throw std::runtime_error("ID_P2P_RELAY processor: Serializer was deleted (this is a bug!)");
				}
				auto connections = wConnections.lock();
				if (!connections)
				{
					assert(false);
					throw std::runtime_error("ID_P2P_RELAY processor: IConnectionManager was deleted (this is a bug!)");
				}

				serializer->deserialize(p->stream, peerId);
				auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(peerId));
				auto connection = connections->getConnection(clientSessionId);
				if (connection)
				{
					p->connection = connection;
				}
				else
				{
					throw std::runtime_error(("ID_P2P_RELAY processor: Connection for peer Id " + clientSessionId + " was not found").c_str());
				}

				return false;
			});

		config.addProcessor((byte)MessageIDTypes::ID_P2P_TUNNEL, [=](Packet_ptr p)
			{
				//_logger->log(LogLevel::Trace, "Received packet from tunnel. Origin: ",std::to_string(p->connection->id()));
				_tunnels->receiveFrom(p->connection->sessionId(), p->stream);
				return true;
			});
	}
}
