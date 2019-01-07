#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/Streams/bytestream.h"
#include "stormancer/IPacketTransform.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/KeyStore.h"

namespace Stormancer
{
	class IAES;
	class Configuration;

	class AESPacketTransform : public IPacketTransform
	{
	public:

#pragma region public_methods

		AESPacketTransform(std::shared_ptr<IAES> aes, std::shared_ptr<Configuration>);

		void onSend(Writer& writer, uint64 peerId, const TransformMetadata& transformMetadata = TransformMetadata()) override;

		void onReceive(ibytestream* stream, uint64 peerId) override;

#pragma endregion

	private:
		std::shared_ptr<IAES> _aes;
		bool _enabled;
	};
}
