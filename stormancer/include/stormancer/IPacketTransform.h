#pragma once

#include "stormancer/headers.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/TransformMetadata.h"

namespace Stormancer
{
	class IPacketTransform
	{
	public:

#pragma region public methods

		virtual ~IPacketTransform()
		{
		}

		virtual void onSend(Writer& writer, const TransformMetadata& transformMetadata = TransformMetadata()) = 0;

		virtual void onReceive(ibytestream* stream, const TransformMetadata& transformMetadata = TransformMetadata()) = 0;

#pragma endregion
	};
}
