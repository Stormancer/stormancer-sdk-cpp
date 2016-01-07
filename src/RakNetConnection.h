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
		RakNetConnection(RakNet::RakNetGUID guid, int64 id, RakNet::RakPeerInterface* peer, std::function<void(RakNetConnection*)> lambdaOnRequestClose);
		virtual ~RakNetConnection();

	public:
		int64 id();
		time_t connectionDate();
		std::string account();
		std::string application();
		ConnectionState state();
		void setConnectionState(ConnectionState connectionState);
		void onConnectionStateChanged(std::function<void(ConnectionState)> callback);
		RakNet::RakNetGUID guid();
		time_t lastActivityDate();
		std::string ipAddress();
		bool operator==(RakNetConnection& other);
		bool operator!=(RakNetConnection& other);
		stringMap& metadata();
		void setMetadata(stringMap& metadata);
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
		std::function<void(std::string)> connectionClosed;
		Action<RakNetConnection*> _closeAction;

	private:
		stringMap _metadata;
		std::string _account;
		std::string _application;
		int64 _id = 0;
		ConnectionState _state = ConnectionState::Disconnected;
		time_t _connectionDate = nowTime_t();
		RakNet::RakPeerInterface* _rakPeer = nullptr;
		RakNet::RakNetGUID _guid;
		time_t _lastActivityDate = nowTime_t();
		std::map<size_t, void*> _localData;
		DependencyResolver* _dependencyResolver = nullptr;
		Action<ConnectionState> _onConnectionStateChanged;
	};
};
