#pragma once
#include "headers.h"
#include "ITransport.h"
#include "ILogger.h"
#include "RaknetConnection.h"
#include <RakPeerInterface.h>

namespace Stormancer
{
	/// Core functions for communicating over the network using RakNet.
	class RakNetTransport : public ITransport
	{
	public:
		RakNetTransport(ILogger* logger, IScheduler* scheduler);
		virtual ~RakNetTransport();

	public:
		void start(std::string type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections = 10, uint16 serverPort = 0);
		pplx::task<IConnection*> connect(std::string endpoint);
		void stop();

	private:
		void initialize(uint16 maxConnections, uint16 serverPort = 0);
		void run();
		void onConnectionIdReceived(uint64 p);
		RakNetConnection* onConnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server);
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
		IScheduler* _scheduler = nullptr;
		Subscription_ptr _scheduledTransportLoop;
		RakNet::SocketDescriptor* _socketDescriptor = nullptr;
	};
};
