#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PPacketDispatcher.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"

namespace Stormancer
{
	P2PPacketDispatcher::P2PPacketDispatcher(std::shared_ptr<P2PTunnels> tunnels, std::shared_ptr<IConnectionManager> connections, std::shared_ptr<ILogger> logger)
		: _tunnels(tunnels)
		, _connections(connections)
		,_logger(logger)
	{
	}

	void P2PPacketDispatcher::registerProcessor(PacketProcessorConfig& config)
	{
		config.addProcessor((byte)MessageIDTypes::ID_P2P_RELAY, new handlerFunction([=](Packet_ptr p) {
			uint64 peerId;
			*p->stream >> peerId;
			auto connection = _connections->getConnection(peerId);
			if (connection)
			{
				p->connection = connection;
			}
			return false;
		}));
		config.addProcessor((byte)MessageIDTypes::ID_P2P_TUNNEL, new handlerFunction([=](Packet_ptr p) {
			//_logger->log(LogLevel::Trace, "Received packet from tunnel. Origin: ",std::to_string(p->connection->id()));
			_tunnels->receiveFrom(p->connection->id(), p->stream);
			return true;
		}));
	}
};
