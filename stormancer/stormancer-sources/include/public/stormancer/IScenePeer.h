#pragma once

#include "stormancer/BuildConfig.h"

#include <string>
#include "stormancer/Streams/bytestream.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/Route.h"
#include "stormancer/Tasks.h"

namespace Stormancer
{
	/// Remote scene
	class IScenePeer
	{
	public:

#pragma region public_methods

		virtual ~IScenePeer() = default;

		virtual uint64 id() const = 0;

		/// Returns the scene handle.
		virtual byte handle() const = 0;

		virtual std::string getSceneId() const = 0;

		virtual std::shared_ptr<IConnection> connection() const = 0;

		virtual const std::unordered_map<std::string, Route_ptr>& routes() const = 0;

		/// Sends a message to the remote scene.
		/// \param routeName Route name.
		/// \param streamWriter function where we write the message in the stream.
		/// \param priority Priority of the message in the network.
		/// \param reliability Reliability behavior of the message in the network.
		virtual void send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "") = 0;

		virtual pplx::task<void> disconnect() = 0;

#pragma endregion
	};
}
