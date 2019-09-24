#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Tasks.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/ConnectionState.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/Event.h"
#include "stormancer/TransformMetadata.h"
#include "rxcpp/rx.hpp"

namespace Stormancer
{
	class ConnectionsRepository;
	class P2PRequestModule;

	/// Interface of a network connection.
	class IConnection
	{
		friend class ConnectionsRepository;
		friend class P2PRequestModule;

	public:

#pragma region public_methods

		virtual ~IConnection() = default;

		/// Sends a system msg to the remote peer.
		/// \param streamWriter A function to write in the stream.
		/// \param priority The priority of the message.
		/// \param reliability The reliability of the message.
		virtual void send(const StreamWriter& streamWriter, int channelUid, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const TransformMetadata& transformMetadata = TransformMetadata()) = 0;

		/// Set the account id and the application name.
		/// \param account The account id.
		/// \param application The application name.
		virtual void setApplication(std::string account, std::string application) = 0;

		/// Close the connection.
		virtual void close(std::string reason = "") = 0;

		/// Returns the ip address of the remote peer.
		virtual std::string ipAddress() const = 0;

		virtual int ping() const = 0;

		///Returns the unique id in the node for the connection.
		virtual uint64 id() const = 0;

		virtual std::string key() const = 0;

		/// Returns the connection date.
		virtual time_t connectionDate() const = 0;

		/// Returns the account id of the application to which this connection is connected.
		virtual const std::string& account() const = 0;

		/// Returns the id of the application to which this connection is connected.
		virtual const std::string& application() const = 0;

		virtual const std::unordered_map<std::string, std::string>& metadata() const = 0;

		virtual std::string metadata(const std::string& key) const = 0;

		virtual void setMetadata(const std::unordered_map<std::string, std::string>& metadata) = 0;

		virtual void setMetadata(const std::string& key, const std::string& value) = 0;

		virtual pplx::task<void> updatePeerMetadata(pplx::cancellation_token /*ct*/ = pplx::cancellation_token::none()) { return pplx::task_from_result(); }

		virtual const DependencyScope& dependencyResolver() const = 0;

		/// Returns the connection state.
		virtual ConnectionState getConnectionState() const = 0;

		virtual rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const = 0;

		virtual pplx::task<void> setTimeout(std::chrono::milliseconds /*timeout*/, pplx::cancellation_token /*ct*/ = pplx::cancellation_token::none()) { return pplx::task_from_result(); }

		virtual std::string sessionId() const = 0;

		Event<std::string> onClose;

#pragma endregion

	protected:

#pragma region protected_methods

		virtual void setSessionId(const std::string& sessionId) = 0;

		virtual void setConnectionState(ConnectionState connectionState) = 0;

#pragma endregion

	};
}
