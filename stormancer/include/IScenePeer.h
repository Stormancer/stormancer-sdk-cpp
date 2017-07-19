#pragma once

#include "headers.h"
#include "bytestream.h"
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
		virtual void send(std::string& routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;

		virtual void disconnect() = 0;

#pragma endregion
	};
};
