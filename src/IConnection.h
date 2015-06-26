#pragma once
#include "headers.h"
#include "ConnectionState.h"
#include <PacketPriority.h>
#include "basic_bytestream.h"

namespace Stormancer
{
	/// Interface of a network connection.
	class IConnection
	{
	public:
	
		/// Constructor.
		IConnection();
		
		/// Destructor.
		virtual ~IConnection();

	public:
		template<typename T>
		void registerComponent(T* component)
		{
			_components[typeid(T).hash_code()] = static_cast<void*>(component);
		}

		template<typename T>
		bool getComponent(T* component = nullptr)
		{
			size_t hash_code = typeid(T).hash_code();
			if (Helpers::mapContains(_components, hash_code))
			{
				if (component != nullptr)
				{
					component = static_cast<T*>(_components[hash_code]);
				}
				return true;
			}

			component = nullptr;
			return false;
		}

		/// Sends a system request to the remote peer.
		/// \param msgId The id of the system message.
		/// \param writer The function to write in the stream.
		virtual void sendSystem(byte msgId, std::function<void(bytestream*)> writer) = 0;
		
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
		virtual void close() = 0;
		
		/// Returns the ip address of the remote peer.
		virtual std::string ipAddress() = 0;
		
		virtual int ping() = 0;
		
		///Returns the unique id in the node for the connection.
		virtual int64 id();
		
		/// Returns the connection date.
		virtual time_t connectionDate();
		
		/// Returns the account id of the application to which this connection is connected.
		virtual std::string account();
		
		/// Returns the id of the application to which this connection is connected.
		virtual std::string application();
		
		/// Returns the connection state.
		virtual ConnectionState state();

	public:
		
		/// Metadata of the connection.
		stringMap metadata;
		
		std::function<void(std::string)> connectionClosed;

	protected:
		ConnectionState _state;
		std::string _account;
		std::string _application;
		int64 _id;
		time_t _connectionDate;
		std::map<size_t, void*> _components;
	};
};
