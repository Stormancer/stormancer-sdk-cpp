#pragma once
#include "headers.h"
#include "ConnectionState.h"
#include <PacketPriority.h>
#include "basic_bytestream.h"
#include "DependencyResolver.h"

namespace Stormancer
{
	/// Interface of a network connection.
	class IConnection
	{
	public:
		/// Sends a system request to the remote peer.
		/// \param msgId The id of the system message.
		/// \param writer The function to write in the stream.
		virtual void sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY) = 0;
		
		/// Sends a scene request to the remote peer.
		/// \param sceneIndex The scene index.
		/// \param route The route handle.
		/// \param writer A function to write in the stream.
		/// \param priority The priority of the message.
		/// \param reliability The reliability of the message.
		virtual void sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;
		
		/// Set the account id and the application name.
		/// \param account The account id.
		/// \param application The application name.
		virtual void setApplication(std::string account, std::string application) = 0;
		
		/// Close the connection.
		virtual void close(std::string reason = "") = 0;
		
		/// Returns the ip address of the remote peer.
		virtual std::string ipAddress() = 0;
		
		virtual int ping() = 0;
		
		///Returns the unique id in the node for the connection.
		virtual int64 id() = 0;
		
		/// Returns the connection date.
		virtual time_t connectionDate() = 0;
		
		/// Returns the account id of the application to which this connection is connected.
		virtual std::string account() = 0;
		
		/// Returns the id of the application to which this connection is connected.
		virtual std::string application() = 0;

		virtual stringMap& metadata() = 0;
		virtual void setMetadata(stringMap& metadata) = 0;
		
		virtual DependencyResolver* dependencyResolver() = 0;

		/// Returns the connection state.
		virtual ConnectionState connectionState() = 0;
		virtual rxcpp::observable<ConnectionState> GetConnectionStateChangedObservable() = 0;

	protected:
		virtual void setConnectionState(ConnectionState connectionState) = 0;
	};
};
