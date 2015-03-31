#pragma once
#include "headers.h"
#include "IConnectionManager.h"
#include "Core/Packet.h"

namespace Stormancer
{
	class ITransport
	{
	public:
		ITransport();
		virtual ~ITransport();

		virtual pplx::task<void> start(wstring name, shared_ptr<IConnectionManager> handler, pplx::cancellation_token token, uint16 port, uint16 maxConnections) = 0;
		virtual pplx::task<shared_ptr<IConnection>> connect(wstring endpoint) = 0;

	public:
		bool isRunning;
		function<void(Packet<>)> packetReceived;
		function<void(shared_ptr<IConnection>)> connectionOpened;
		function<void(shared_ptr<IConnection>)> connectionClosed;
		wstring name;
		uint64 id;
	};
};
