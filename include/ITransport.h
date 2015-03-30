#pragma once
#include "headers.h"
#include "IConnectionManager.h"

namespace Stormancer
{
	class ITransport
	{
	public:
		ITransport();
		virtual ~ITransport();

		virtual task<void> start(wstring name, shared_ptr<IConnectionManager> handler, cancellation_token token, uint16 port, uint16 maxConnections) = 0;
		virtual task<IConnection> connect(wstring endpoint) = 0;

	public:
		bool isRunning;
		function<void(Packet<>)> packetReceived;
		function<void(shared_ptr<IConnection>)> connectionOpened;
		function<void(shared_ptr<IConnection>)> connectionClosed;
		wstring name;
		uint64 id;
	};
};
