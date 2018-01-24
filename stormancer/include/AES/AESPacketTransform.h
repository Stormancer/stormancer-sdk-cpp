#pragma once

#include "headers.h"
#include "Streams/bytestream.h"
#include "IPacketTransform.h"
#include "TransformMetadata.h"

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
