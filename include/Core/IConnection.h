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
		void registerComponent(T component);

		template<typename T>
		T getComponent();

		virtual void sendSystem(char msgId, function<void(byteStream)> writer) = 0;
		virtual void sendToScene(char sceneIndex, uint16 route, function<void(byteStream)> writer, PacketPriority priority, PacketReliability reliability) = 0;
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
	};
};
