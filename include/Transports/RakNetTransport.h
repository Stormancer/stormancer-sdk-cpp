#pragma once
#include "headers.h"
#include "ITransport.h"
#include "ILogger.h"
#include "Transports/RaknetConnection.h"
#include <RakPeerInterface.h>

namespace Stormancer
{
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
		void onConnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server);
		void onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, wstring reason);
		void onMessageReceived(RakNet::Packet* packet);
		RakNetConnection* getConnection(RakNet::RakNetGUID guid);
		RakNetConnection* createNewConnection(RakNet::RakNetGUID raknetGuid, RakNet::RakPeerInterface* peer);
		RakNetConnection* removeConnection(RakNet::RakNetGUID guid);
		void onRequestClose(RakNetConnection* c);

	private:
		IConnectionManager* _handler;
		RakNet::RakPeerInterface* _peer;
		ILogger* _logger;
		wstring _type;
		map<uint64, RakNetConnection*> _connections;
		const int connectionTimeout = 5000;
		map<wstring, pplx::task_completion_event<IConnection*>> _pendingConnections;
	};
};
