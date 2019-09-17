#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/IConnectionManager.h"
#include "stormancer/Logger/ILogger.h"
#include <unordered_map>

namespace Stormancer
{
	class ConnectionsRepository : public IConnectionManager, public std::enable_shared_from_this<ConnectionsRepository>
	{
	public:

		ConnectionsRepository(ILogger_ptr logger);
		pplx::task<std::shared_ptr<IConnection>> addPendingConnection( std::string clientSessionId) override;
		void newConnection(std::shared_ptr<IConnection> connection) override;
		std::shared_ptr<IConnection> getConnection(std::string id) override;
		
		void closeConnection(std::shared_ptr<IConnection> connection, const std::string& reason) override;
		pplx::task<void> closeAllConnections(const std::string& reason) override;
		pplx::task<std::shared_ptr<IConnection>> getConnection(std::string id,std::function<pplx::task<std::shared_ptr<IConnection>>(std::string)> connectionFactory) override;
		int getConnectionCount() override;
		pplx::task<void> setTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct) override;

	private:

		class PendingConnection
		{
		public:
			
			
			std::string sessionId;
			pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
		};

		struct ConnectionContainer
		{
			std::shared_ptr<IConnection> connection;
			Event<std::string>::Subscription onCloseSubscription;
		};

		std::unordered_map<std::string, pplx::task<ConnectionContainer>> _connectionsByKey;
		std::unordered_map<std::string, pplx::task<ConnectionContainer>> _connections;
		std::unordered_map<std::string, PendingConnection> _pendingP2PConnections;
		ILogger_ptr _logger;
		std::mutex _mutex;
	};
}
