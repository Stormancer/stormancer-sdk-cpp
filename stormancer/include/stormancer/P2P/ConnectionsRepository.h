#pragma once

#include "stormancer/headers.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	class ConnectionsRepository : public IConnectionManager, public std::enable_shared_from_this<ConnectionsRepository>
	{
	public:

		ConnectionsRepository(ILogger_ptr logger);
		pplx::task<std::shared_ptr<IConnection>> addPendingConnection(uint64 id) override;
		void newConnection(std::shared_ptr<IConnection> connection) override;
		std::shared_ptr<IConnection> getConnection(uint64 id) override;
		std::shared_ptr<IConnection> getConnection(std::string id) override;
		void closeConnection(std::shared_ptr<IConnection> connection, const std::string& reason) override;
		pplx::task<void> closeAllConnections(const std::string& reason) override;
		pplx::task<std::shared_ptr<IConnection>> getConnection(std::string id,std::function<pplx::task<std::shared_ptr<IConnection>>(std::string)> connectionFactory) override;
		int getConnectionCount() override;

	private:

		class PendingConnection
		{
		public:
			
			uint64 id;
			pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
		};
		std::unordered_map<std::string, pplx::task<std::shared_ptr<IConnection>>> _connectionsByKey;
		std::unordered_map<uint64, pplx::task<std::shared_ptr<IConnection>>> _connections;
		std::unordered_map<uint64, PendingConnection> _pendingP2PConnections;
		ILogger_ptr _logger;
		std::mutex _mutex;
	};
}
