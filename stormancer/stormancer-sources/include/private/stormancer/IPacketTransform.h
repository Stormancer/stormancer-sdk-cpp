#pragma once

#include "stormancer/BuildConfig.h"


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

		virtual void onSend(StreamWriter& streamWriter, uint64 peerId, const TransformMetadata& transformMetadata = TransformMetadata()) = 0;

		virtual void onReceive(ibytestream& stream, uint64 peerId) = 0;

#pragma endregion
	};
}
