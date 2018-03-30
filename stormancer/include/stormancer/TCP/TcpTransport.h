#pragma once

#include <thread>
#include "SocketDefines.h"
#include "stormancer/headers.h"
#include "stormancer/ITransport.h"
#include "stormancer/TCP/TcpConnection.h"
#include "stormancer/IScheduler.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	class TcpTransport : public ITransport
	{
	public:

#pragma region public_methods

		TcpTransport(DependencyResolver* resolver);
		~TcpTransport();
		void start(std::string type, std::shared_ptr<IConnectionManager> handler, pplx::cancellation_token token = pplx::cancellation_token::none(), uint16 port = 0, uint16 maxConnections = 0) override;
		pplx::task<std::shared_ptr<IConnection>> connect(std::string endpoint) override;
		bool isRunning() const override;
		std::string name() const override;
		uint64 id() const override;
		DependencyResolver* dependencyResolver() const override;
		void onPacketReceived(std::function<void(Packet_ptr)> callback) override;
		std::string host() const override;
		uint16 port() const override;
		std::vector<std::string> externalAddresses() const override;
		//P2P (Not implemented)
		pplx::task<int> sendPing(const std::string& address) override;
		void openNat(const std::string& address) override;
		std::vector<std::string> getAvailableEndpoints() const override;

#pragma endregion

	private:

#pragma region private_methods

		void stop();
		void initialize();
		void run();
		void onConnectionIdReceived(int64 id);
		void listen(std::string endpoint);
		std::shared_ptr<TcpConnection> createNewConnection(std::string endpoint);
		void onDisconnection(std::string endpoint, std::string reason);

#pragma endregion

#pragma region private_members

		bool _isRunning = false;
		DependencyResolver* _dependencyResolver = nullptr;
		std::shared_ptr<IConnectionManager> _handler;
		std::shared_ptr<ILogger> _logger;
		std::shared_ptr<TcpConnection> _connection;
		std::unordered_map<std::string, pplx::task_completion_event<std::weak_ptr<IConnection>>> _pendingConnections;
		std::shared_ptr<IScheduler> _scheduler;
		std::mutex _mutex;
		std::mutex _packetQueueMutex;
		std::queue<Packet_ptr> _pendingPackets;
		std::string _name;
		std::string _type;
		std::shared_ptr<SOCKET> _socketId;
		Action<Packet_ptr> _onPacketReceived;
		uint64 _id = 0;
		std::string _host;
		uint16 _port = 0;
		std::shared_ptr<std::thread> _listeningThread;
		std::mutex _actionQueueMutex;
		std::queue<std::function<void()>> _actionQueue; 

#pragma endregion
	};
}