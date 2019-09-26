#pragma once

#include "stormancer/BuildConfig.h"

#include "RakPeerInterface.h"

#include "stormancer/ITransport.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/RakNet/RakNetConnection.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/IScheduler.h"

namespace Stormancer
{
	/// Core functions for communicating over the network using RakNet.

	struct ConnectionRequest
	{
		pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
		std::string endpoint;
		pplx::cancellation_token cancellationToken = pplx::cancellation_token::none();
		std::string p2pSessionId;
		std::string peerSessionId;
		std::string parentId;
		bool isP2P()
		{
			return parentId != "";
		}
	};

	class RakNetTransport : public ITransport, public std::enable_shared_from_this<RakNetTransport>
	{
	public:

#pragma region public_methods

		RakNetTransport(const DependencyScope& scope);
		~RakNetTransport();
		void start(std::string type, std::shared_ptr<IConnectionManager> handler, const DependencyScope& parentScope, pplx::cancellation_token ct = pplx::cancellation_token::none(), uint16 maxConnections = 10, uint16 serverPort = 0) override;
		pplx::task<std::shared_ptr<IConnection>> connect(std::string endpoint,std::string id, std::string parentId, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		bool isRunning() const override;
		std::string name() const override;
		
		const DependencyScope& dependencyResolver() const override;
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
		void initialize(uint16 maxConnections, uint16 serverPort, const DependencyScope& parentScope);
		void run();
		// This overload sets the request as completed after it has updated its ConnectionState.
		std::shared_ptr<RakNetConnection> onConnection(RakNet::SystemAddress systemAddress, RakNet::RakNetGUID guid, std::string peerId, const ConnectionRequest& request);
		// This overolad is for connections that do not have an associated ConnectionRequest.
		std::shared_ptr<RakNetConnection> onConnection(RakNet::SystemAddress systemAddress, RakNet::RakNetGUID guid, std::string remotePeerSessionId, std::string connectionId);
		void onDisconnection(uint64 guid, std::string reason);
		void onMessageReceived(RakNet::Packet* packet);
		std::shared_ptr<RakNetConnection> getConnection(uint64 guid);
		std::shared_ptr<RakNetConnection> createNewConnection(RakNet::RakNetGUID raknetGuid, std::string peerId, std::string key);
		std::shared_ptr<RakNetConnection> removeConnection(uint64 guid);
		pplx::task<int> sendPing(const std::string& address, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		pplx::task<bool> sendPingImplTask(const std::string& address);
		bool sendPingImpl(const std::string& address);
		pplx::task<int> sendPing(const std::string& address, const int nb, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		void openNat(const std::string& address) override;
		std::vector<std::string> getAvailableEndpoints() const override;
		void startNextPendingConnections();

#pragma endregion

#pragma region private_members
		struct ConnectionContainer
		{
			std::shared_ptr<RakNetConnection> connection;
			Event<std::string>::Subscription onCloseSubscription;
		};
		struct PendingPing
		{
			PendingPing(const std::string address, const pplx::task_completion_event<bool> tce)
				: address(address),
				tce(tce)
			{};

			const std::string address;
			const pplx::task_completion_event<bool> tce;
		};

		bool _isRunning = false;
		DependencyScope _dependencyScope;
		std::shared_ptr<IConnectionManager> _handler;
		std::shared_ptr<RakNet::RakPeerInterface> _peer;
		std::shared_ptr<ILogger> _logger;
		std::string _type;
		std::unordered_map<uint64, ConnectionContainer> _connections;
		std::mutex _pendingConnection_mtx;
		std::queue<ConnectionRequest> _pendingConnections;
		std::shared_ptr<IScheduler> _scheduler;
		std::shared_ptr<RakNet::SocketDescriptor> _socketDescriptor;
		std::recursive_mutex _mutex;
		std::string _name = "raknet";
		
		Action<Packet_ptr> _onPacketReceived;
		std::string _host;
		uint16 _port = 0;
		
		std::mutex _pendingPingsMutex;
		std::unordered_map<std::string, pplx::task_completion_event<int>> _pendingPings;

		std::mutex _pendingPingsQueueMutex;
		std::queue<PendingPing> _pendingPingsQueue;

#pragma endregion
	};
}
