#pragma once
#include "headers.h"
#include "IConnection.h"
#include <RakPeerInterface.h>
#include <RakNetTypes.h>

namespace Stormancer
{
	/// Connection to the network using RakNet.
	class RakNetConnection : public IConnection
	{
	public:
	
		/// Constructor.
		RakNetConnection(RakNet::RakNetGUID guid, int64 id, RakNet::RakPeerInterface* peer, std::function<void(RakNetConnection*)> lambdaOnRequestClose);
		
		/// Destructor.
		virtual ~RakNetConnection();

	public:
	
		/// Returns the id of the connection.
		RakNet::RakNetGUID guid();
		
		/// Returns the last activity date.
		time_t lastActivityDate();
		
		/// Returns the ip address of the connection.
		std::string ipAddress();
		
		/// Comparison operator
		bool operator==(RakNetConnection& other);
		
		/// Comparison operator (non)
		bool operator!=(RakNetConnection& other);
		
		/// Returns the metadatas
		stringMap metadata();

		DependencyResolver* dependencyResolver();
		
		void close();

		void sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY);

		void sendRaw(std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability, char channel);

		void sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability);

		int ping();

		void setApplication(std::string account, std::string application);

		template<typename T>
		void registerComponent(T* component)
		{
			_localData[typeid(T).hash_code()] = static_cast<void*>(component);
		}

		template<typename T>
		bool getComponent(T* component = nullptr)
		{
			size_t hash_code = typeid(T).hash_code();
			if (mapContains(_localData, hash_code))
			{
				if (component != nullptr)
				{
					component = static_cast<T*>(_localData[hash_code]);
				}
				return true;
			}

			component = nullptr;
			return false;
		}

	public:
		Action<RakNetConnection*> _closeAction;

	private:
		RakNet::RakPeerInterface* _rakPeer;
		RakNet::RakNetGUID _guid;
		stringMap _metadata;
		time_t _lastActivityDate;
		std::map<size_t, void*> _localData;
		DependencyResolver* _dependencyResolver = nullptr;
	};
};
