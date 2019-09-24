#include "stormancer/stdafx.h"
#include "stormancer/PacketTransformProcessor.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	PacketTransformProcessor::PacketTransformProcessor(std::shared_ptr<AESPacketTransform> packetTransform)
		:_packetTransform(packetTransform)
	{
	}

	void PacketTransformProcessor::registerProcessor(PacketProcessorConfig& config)
	{
		std::weak_ptr<AESPacketTransform> transform = _packetTransform;
		config.addProcessor((byte)MessageIDTypes::ID_ENCRYPTED, [transform](Packet_ptr p)
		{
			if (auto t = transform.lock())
			{
				t->onReceive(p->stream, p->connection->id());
			}
			return false;
		});
	}
}
