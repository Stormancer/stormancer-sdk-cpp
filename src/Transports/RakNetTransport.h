#pragma once
#include "headers.h"
#include "ITransport.h"
#include "ILogger.h"
#include "Transports/RaknetConnection.h"
#include <RakPeerInterface.h>

namespace Stormancer
{
	/// Core functions for communicating over the network using RakNet.
	class RakNetTransport : public ITransport
	{
	public:
		RakNetTransport(ILogger* logger);
		virtual ~RakNetTransport();

	public:
		pplx::task<void> start(std::string type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections = 11, uint16 serverPort = 0);
		pplx::task<IConnection*> connect(std::string endpoint);

	private:
		void run(pplx::cancellation_token token, pplx::task_completion_event<void> startupTce, uint16 maxConnections = 11, uint16 serverPort = 0);
		void onConnectionIdReceived(uint64 p);
		void onConnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server);
		void onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, std::string reason);
		void onMessageReceived(RakNet::Packet* packet);
		RakNetConnection* getConnection(RakNet::RakNetGUID guid);
		RakNetConnection* createNewConnection(RakNet::RakNetGUID raknetGuid, RakNet::RakPeerInterface* peer);
		RakNetConnection* removeConnection(RakNet::RakNetGUID guid);
		void onRequestClose(RakNetConnection* c);

	private:
		IConnectionManager* _handler = nullptr;
		RakNet::RakPeerInterface* _peer = nullptr;
		ILogger* _logger = nullptr;
		std::string _type;
		std::map<uint64, RakNetConnection*> _connections;
		const int connectionTimeout = 5000;
		std::map<std::string, pplx::task_completion_event<IConnection*>> _pendingConnections;
	};
};
