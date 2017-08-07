#pragma once

#include "headers.h"
#include "IConnection.h"

namespace Stormancer
{
	/// Manages the network connections.
	class IConnectionManager
	{
	public:

#pragma region public_methods

		virtual pplx::task<std::shared_ptr<IConnection>> addPendingConnection(uint64 id) = 0;
		virtual void newConnection(std::shared_ptr<IConnection> connection) = 0;
		virtual void closeConnection(std::shared_ptr<IConnection> connection, const std::string& reason) = 0;
		virtual std::shared_ptr<IConnection> getConnection(uint64 id) = 0;
		virtual int getConnectionCount() = 0;

#pragma endregion
	};
};
