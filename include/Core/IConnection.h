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

	public:
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
		virtual wstring ipAddress() = 0;
		virtual int ping() = 0;
		virtual int64 id();
		virtual time_t connectionDate();
		virtual wstring account();
		virtual wstring application();
		virtual ConnectionState state();

	public:
		stringMap metadata;
		function<void(string)> connectionClosed;

	protected:
		ConnectionState _state;
		wstring _account;
		wstring _application;
		int64 _id;
		time_t _connectionDate;
		map<size_t, void*> _components;
	};
};
