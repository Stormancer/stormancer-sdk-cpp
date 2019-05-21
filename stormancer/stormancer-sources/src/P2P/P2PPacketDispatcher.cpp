#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PPacketDispatcher.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"

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
			uint64 peerId;
			auto serializer = wSerializer.lock();
			if (!serializer)
			{
				return false;
			}
			auto connections = wConnections.lock();
			if (!connections)
			{
				return false;
			}

			serializer->deserialize(p->stream, peerId);

			auto connection = connections->getConnection(peerId);
			if (connection)
			{
				p->connection = connection;
			}

			return false;
		});

		config.addProcessor((byte)MessageIDTypes::ID_P2P_TUNNEL, [=](Packet_ptr p)
		{
			//_logger->log(LogLevel::Trace, "Received packet from tunnel. Origin: ",std::to_string(p->connection->id()));
			_tunnels->receiveFrom(p->connection->id(), p->stream);
			return true;
		});
	}
}
