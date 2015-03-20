#pragma once
#include "libs.h"
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

		template<typename T> virtual void registerComponent(T component) = 0;
		template<typename T> virtual T getComponent() = 0;
		virtual void close() = 0;
		virtual void sendSystem(char msgId, function < void(stringbuf) > writer) = 0;
		virtual void sendToScene(char sceneIndex, uint16_t route, function < void(stringbuf) > writer, PacketPriority priority, PacketReliability reliability) = 0;
		virtual function < void(string) > connectionClosed() = 0;
		virtual void setApplication(string account, string application);

	public:
		uint64_t id;
		string ipAddress;
		time_t connectionDate;
		StringMap metadata;
		string account;
		string application;
		ConnectionState state;
		uint32_t ping;
	};
};
