#pragma once

#include "stormancer/headers.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/IPacketTransform.h"
#include "stormancer/TransformMetadata.h"

namespace Stormancer
{
	class AESPacketTransform : public IPacketTransform
	{
	public:

#pragma region public_methods

		AESPacketTransform();

		void onSend(Writer& writer, const TransformMetadata& transformMetadata = TransformMetadata()) override;

		void onReceive(ibytestream* stream, const TransformMetadata& transformMetadata = TransformMetadata()) override;

#pragma endregion
	};
}
