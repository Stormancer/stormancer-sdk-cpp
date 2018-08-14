#pragma once

#include "stormancer/headers.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/ConnectionState.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/DependencyResolver.h"
#include "stormancer/Action.h"
#include "stormancer/ChannelUidStore.h"
#include "stormancer/TransformMetadata.h"

namespace Stormancer
{
	/// Interface of a network connection.
	class IConnection
	{
	public:

#pragma region public_methods

		virtual ~IConnection()
		{
		}

		/// Sends a system msg to the remote peer.
		/// \param writer A function to write in the stream.
		/// \param priority The priority of the message.
		/// \param reliability The reliability of the message.
		virtual void send(const Writer& writer, int channelUid, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const TransformMetadata& transformMetadata = TransformMetadata()) = 0;
		
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
		
		/// Returns the connection date.
		virtual time_t connectionDate() const = 0;
		
		/// Returns the account id of the application to which this connection is connected.
		virtual const std::string& account() const = 0;
		
		/// Returns the id of the application to which this connection is connected.
		virtual const std::string& application() const = 0;

		virtual const std::map<std::string, std::string>& metadata() const = 0;
		virtual std::string metadata(const std::string& key) const = 0;
		virtual void setMetadata(const std::map<std::string, std::string>& metadata) = 0;
		virtual void setMetadata(const std::string& key, const std::string& value) = 0;
		
		virtual std::weak_ptr<DependencyResolver> dependencyResolver() = 0;

		/// Returns the connection state.
		virtual ConnectionState getConnectionState() const = 0;
		virtual rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const = 0;

		virtual Action<std::string>::TIterator onClose(std::function<void(std::string)> callback) = 0;
		virtual Action<std::string>& onCloseAction() = 0;

		ChannelUidStore& getChannelUidStore()
		{
			return _channelUidStore;
		}

#pragma endregion

	protected:

#pragma region protected_methods

		virtual void setConnectionState(ConnectionState connectionState) = 0;

#pragma endregion

#pragma region protected_members

		ChannelUidStore _channelUidStore;

#pragma endregion
	};
};
