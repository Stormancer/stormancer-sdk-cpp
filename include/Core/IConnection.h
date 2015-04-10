#pragma once
#include "headers.h"
#include "ConnectionState.h"
#include "PacketPriority.h"
#include "PacketReliability.h"

namespace Stormancer
{
	class IConnection
	{
	public:
		IConnection();
		virtual ~IConnection();

		template<typename T>
		void registerComponent(T* component)
		{
			_components[typeid(T).hash_code()] = (void*)component;
		}

		template<typename T>
		bool getComponent(T* component = nullptr)
		{
			size_t hash_code = typeid(T).hash_code();
			if (Helpers::mapContains(_components, hash_code))
			{
				if (component != nullptr)
				{
					component = (T*)(_components[hash_code]);
				}
				return true;
			}

			component = nullptr;
			return false;
		}

		virtual void sendSystem(char msgId, function<void(byteStream*)> writer) = 0;
		virtual void sendToScene(char sceneIndex, uint16 route, function<void(byteStream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;
		virtual void setApplication(wstring account, wstring application) = 0;
		virtual void close() = 0;

	public:
		uint64_t id;
		wstring ipAddress;
		time_t connectionDate;
		stringMap metadata;
		wstring account;
		wstring application;
		ConnectionState state;
		int ping;
		function<void(string)> connectionClosed;

	private:
		map<size_t, void*> _components;
	};
};
