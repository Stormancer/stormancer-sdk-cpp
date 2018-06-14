#pragma once

#include "stormancer/headers.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/IPacketTransform.h"
#include "stormancer/TransformMetadata.h"
#include "stormancer/KeyStore.h"

namespace Stormancer
{
	class IAES;

	class AESPacketTransform : public IPacketTransform
	{
	public:

#pragma region public_methods

		AESPacketTransform(std::shared_ptr<IAES> aes);

		void onSend(Writer& writer, uint64 peerId, const TransformMetadata& transformMetadata = TransformMetadata()) override;

		void onReceive(ibytestream* stream, uint64 peerId, const TransformMetadata& transformMetadata = TransformMetadata()) override;

#pragma endregion

	private:
		std::shared_ptr<IAES> _aes;
	};
}
