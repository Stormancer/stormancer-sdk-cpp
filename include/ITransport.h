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

		virtual pplx::task<void> start(wstring name, IConnectionManager* handler, pplx::cancellation_token token, uint16 port, uint16 maxConnections) = 0;
		virtual pplx::task<IConnection*> connect(wstring endpoint) = 0;

	public:
		bool isRunning;
		function<void(Packet<>*)> packetReceived;
		function<void(IConnection*)> connectionOpened;
		function<void(IConnection*)> connectionClosed;
		wstring name;
		uint64 id;
	};
};
