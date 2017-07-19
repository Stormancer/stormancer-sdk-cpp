#pragma once

#include "headers.h"
#include "PacketPriority.h"
#include "ConnectionState.h"
#include "bytestream.h"
#include "DependencyResolver.h"
#include "Action.h"

namespace Stormancer
{
	/// Interface of a network connection.
	class IConnection
	{
	public:

#pragma region public_methods

		virtual ~IConnection() {}

		/// Sends a system msg to the remote peer.
		/// \param msgId The id of the system message.
		/// \param writer The function to write in the stream.
		virtual void sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY) = 0;
		
		/// Sends a scene msg to the remote peer.
		/// \param sceneIndex The scene index.
		/// \param route The route handle.
		/// \param writer A function to write in the stream.
		/// \param priority The priority of the message.
		/// \param reliability The reliability of the message.
		virtual void sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;
		
		/// Sends a raw msg to the remote peer.
		/// \param msgId The id of the system message.
		/// \param writer The function to write in the stream.
		virtual void sendRaw(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;

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
		
		virtual DependencyResolver* dependencyResolver() = 0;

		/// Returns the connection state.
		virtual ConnectionState getConnectionState() const = 0;
		virtual rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const = 0;

#pragma endregion

	protected:

#pragma region protected_methods

		virtual void setConnectionState(ConnectionState connectionState) = 0;

#pragma endregion
	};
};
