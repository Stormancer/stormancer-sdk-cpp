#pragma once

#include "headers.h"
#include "IConnectionManager.h"

namespace Stormancer
{
	class ConnectionsRepository : public IConnectionManager
	{
	public:

		ConnectionsRepository();
		pplx::task<std::shared_ptr<IConnection>> addPendingConnection(const std::string& address, uint64 id) override;
		uint64 generateNewConnectionId(const std::string& address) override;
		void newConnection(std::shared_ptr<IConnection> connection) override;
		std::shared_ptr<IConnection> getConnection(uint64 id) override;
		void closeConnection(std::shared_ptr<IConnection> connection, const std::string& reason) override;
		int getConnectionCount() override;

	private:

		class PendingConnection
		{
		public:
			uint64 id;
			pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
		};

		std::unordered_map<uint64, std::shared_ptr<IConnection>> _connections;
		std::unordered_map<std::string, PendingConnection> _pendingP2PConnections;
	};
}
