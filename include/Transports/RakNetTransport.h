#pragma once
#include "headers.h"
#include "ITransport.h"
#include "Core/ILogger.h"
#include "Transports/RaknetConnection.h"
#include <RakPeerInterface.h>

namespace Stormancer
{
	using namespace RakNet;

	class RakNetTransport : public ITransport
	{
	public:
		RakNetTransport(ILogger* logger);
		virtual ~RakNetTransport();

	public:
		pplx::task<void> start(wstring type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections = 11, uint16 serverPort = 0);
		pplx::task<IConnection*> connect(wstring endpoint);

	private:
		void run(pplx::cancellation_token token, pplx::task_completion_event<void> startupTce, uint16 maxConnections = 11, uint16 serverPort = 0);
		void onConnectionIdReceived(uint64 p);

	private:
		IConnectionManager* _handler;
		RakPeerInterface* _peer;
		ILogger* _logger;
		wstring _type;
		map<uint64, RakNetConnection*> _connections;
		const int connectionTimeout = 5000;
		map<wstring, pplx::task_completion_event<IConnection*>> _pendingConnections;
	};
};
