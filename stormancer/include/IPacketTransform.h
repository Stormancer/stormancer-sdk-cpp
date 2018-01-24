#pragma once

#include "headers.h"
#include "Streams/bytestream.h"
#include "TransformMetadata.h"

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
