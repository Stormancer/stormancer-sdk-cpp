#pragma once

#include "RakPeerInterface.h"
#include "stormancer/headers.h"
#include "stormancer/ITransport.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/RakNet/RaknetConnection.h"
#include "stormancer/DependencyResolver.h"
#include "stormancer/IScheduler.h"

namespace Stormancer
{
	/// Core functions for communicating over the network using RakNet.

	class ConnectionRequest
	{
	public:
		pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
		std::string endpoint;
	};

	class RakNetTransport : public ITransport
	{
	public:

#pragma region public_methods

		RakNetTransport(DependencyResolver* resolver);
		~RakNetTransport();
		void start(std::string type, std::shared_ptr<IConnectionManager> handler, pplx::cancellation_token token = pplx::cancellation_token::none(), uint16 maxConnections = 10, uint16 serverPort = 0) override;
		pplx::task<std::shared_ptr<IConnection>> connect(std::string endpoint) override;
		bool isRunning() const override;
		std::string name() const override;
		uint64 id() const override;
		DependencyResolver* dependencyResolver() const override;
		void onPacketReceived(std::function<void(Packet_ptr)> callback) override;
		std::string host() const override;
		uint16 port() const override;

		/// Get the addresses (format: ip|port) used by this RakNet peer to receive messages,
		/// both local and external (if the peer is already connected to the server).
		std::vector<std::string> externalAddresses() const override;

	private:

#pragma endregion

#pragma region private_methods

		void stop();
		void initialize(uint16 maxConnections, uint16 serverPort = 0);
		void run();
		void onConnectionIdReceived(uint64 p);
		std::shared_ptr<RakNetConnection> onConnection(RakNet::SystemAddress systemAddress, RakNet::RakNetGUID guid, uint64 peerId);
		void onDisconnection(RakNet::Packet* packet, std::string reason);
		void onMessageReceived(RakNet::Packet* packet);
		std::shared_ptr<RakNetConnection> getConnection(RakNet::RakNetGUID guid);
		std::shared_ptr<RakNetConnection> createNewConnection(RakNet::RakNetGUID raknetGuid, uint64 peerId);
		std::shared_ptr<RakNetConnection> removeConnection(RakNet::RakNetGUID guid);
		pplx::task<int> sendPing(const std::string& address) override;
		bool sendPingImpl(const std::string& address);
		pplx::task<int> sendPing(const std::string& address, const int nb) override;
		void openNat(const std::string& address) override;
		std::vector<std::string> getAvailableEndpoints() const override;
		void startNextPendingConnections();
#pragma endregion

#pragma region private_members

		bool _isRunning = false;
		DependencyResolver* _dependencyResolver = nullptr;
		std::shared_ptr<IConnectionManager> _handler;
		std::shared_ptr<RakNet::RakPeerInterface> _peer;
		std::shared_ptr<ILogger> _logger;
		std::string _type;
		std::map<uint64, std::shared_ptr<RakNetConnection>> _connections;
		std::mutex _pendingConnection_mtx;
		std::queue<ConnectionRequest> _pendingConnections;
		std::shared_ptr<IScheduler> _scheduler;
		std::shared_ptr<RakNet::SocketDescriptor> _socketDescriptor;
		std::mutex _mutex;
		std::string _name = "raknet";
		uint64 _id = 0;
		Action<Packet_ptr> _onPacketReceived;
		std::string _host;
		uint16 _port = 0;
		RakNet::RakNetGUID _serverRakNetGUID;
		bool _serverConnected = false;
		std::mutex _pendingPingsMutex;
		std::unordered_map<std::string, pplx::task_completion_event<int>> _pendingPings;
		

#pragma endregion
	};
};
