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

		virtual pplx::task<void> start(wstring name, IConnectionManager* handler, pplx::cancellation_token token, uint16 port = 0, uint16 maxConnections = 0) = 0;
		virtual pplx::task<IConnection*> connect(wstring endpoint) = 0;
		bool isRunning();
		wstring name();
		uint64 id();

	public:
		vector<function<void(Packet<>*)>> packetReceived;
		vector<function<void(IConnection*)>> connectionOpened;
		vector<function<void(IConnection*)>> connectionClosed;

	protected:
		bool _isRunning;
		wstring _name;
		uint64 _id;
	};
};
