#pragma once
#include "stormancer/IPacketProcessor.h"
#include "stormancer/AES/AESPacketTransform.h"

namespace Stormancer
{
	class PacketTransformProcessor : public IPacketProcessor, public std::enable_shared_from_this<PacketTransformProcessor>
	{
	public:
		PacketTransformProcessor(std::shared_ptr<AESPacketTransform> packetTransform);

		void registerProcessor(PacketProcessorConfig& config) override;

	private:
		std::shared_ptr<AESPacketTransform> _packetTransform;
	};
}