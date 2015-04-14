#pragma once
#include "headers.h"
#include "Core/IConnection.h"
#include <RakPeerInterface.h>
#include <RakNetTypes.h>

namespace Stormancer
{
	using namespace RakNet;

	class RakNetConnection : public IConnection
	{

	public:
		RakNetConnection(RakNetGUID guid, int64 id, RakPeerInterface* peer, function<void(RakNetConnection*)> closeAction);
		virtual ~RakNetConnection();

	public:
		RakNetGUID guid();
		time_t lastActivityDate();
		wstring ipAddress();
		bool operator==(RakNetConnection& other);
		bool operator!=(RakNetConnection& other);
		stringMap metadata();
		void close();
		void sendSystem(byte msgId, function<void(BitStream*)> writer);
		void sendRaw(function<void(BitStream*)> writer, PacketPriority priority, PacketReliability reliability, char channel);
		void sendToScene(byte sceneIndex, uint16 route, function<void(BitStream*)> writer, PacketPriority priority, PacketReliability reliability);
		int ping();

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

	private:
		RakPeerInterface* _rakPeer;
		RakNetGUID _guid;
		function<void(RakNetConnection*)> _closeAction;
		stringMap _metadata;
		time_t _lastActivityDate;
		map<size_t, void*> _localData;
	};
};
