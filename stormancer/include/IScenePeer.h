#pragma once

#include "headers.h"
#include "Streams/bytestream.h"
#include "PacketPriority.h"

namespace Stormancer
{
	/// Remote scene
	class IScenePeer
	{
	public:

#pragma region public_methods

		virtual ~IScenePeer() {}
	
		virtual uint64 id() = 0;

		/// Sends a message to the remote scene.
		/// \param routeName Route name.
		/// \param writer function where we write the message in the stream.
		/// \param priority Priority of the message in the network.
		/// \param reliability Reliability behavior of the message in the network.
		virtual void send(const std::string& routeName, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED) = 0;

		virtual void disconnect() = 0;

#pragma endregion
	};
};
