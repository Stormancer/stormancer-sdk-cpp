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
		RakNetConnection(RakNet::RakNetGUID guid, int64 id, RakNet::RakPeerInterface* peer, function<void(RakNetConnection*)>* lambdaOnRequestClose);
		
		/// Destructor.
		virtual ~RakNetConnection();

	public:
	
		/// Returns the id of the connection.
		RakNet::RakNetGUID guid();
		
		/// Returns the last activity date.
		time_t lastActivityDate();
		
		/// Returns the ip address of the connection.
		wstring ipAddress();
		
		/// Comparison operator
		bool operator==(RakNetConnection& other);
		
		/// Comparison operator (non)
		bool operator!=(RakNetConnection& other);
		
		/// Returns the metadatas
		stringMap metadata();
		
		
		void close();
		void sendSystem(byte msgId, function<void(bytestream*)> writer);
		void sendRaw(function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability, char channel);
		void sendToScene(byte sceneIndex, uint16 route, function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability);
		int ping();
		void setApplication(wstring account, wstring application);

		template<typename T>
		void registerComponent(T* component)
		{
			_localData[typeid(T).hash_code()] = static_cast<void*>(component);
		}

		template<typename T>
		bool getComponent(T* component = nullptr)
		{
			size_t hash_code = typeid(T).hash_code();
			if (Helpers::mapContains(_localData, hash_code))
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
		map<size_t, void*> _localData;
	};
};
