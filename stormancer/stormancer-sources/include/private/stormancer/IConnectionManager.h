#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/IConnection.h"
#include "stormancer/StormancerTypes.h"

namespace Stormancer
{
	/// Manages the network connections.
	class IConnectionManager
	{
	public:

#pragma region public_methods

		virtual ~IConnectionManager() = default;

		virtual pplx::task<std::shared_ptr<IConnection>> addPendingConnection(uint64 id, std::string clientSessionId) = 0;
		virtual void newConnection(std::shared_ptr<IConnection> connection) = 0;
		virtual void closeConnection(std::shared_ptr<IConnection> connection, const std::string& reason) = 0;
		virtual std::shared_ptr<IConnection> getConnection(uint64 id) = 0;
		virtual std::shared_ptr<IConnection> getConnection(std::string id) = 0;
		virtual pplx::task<std::shared_ptr<IConnection>> getConnection(std::string id, std::function<pplx::task<std::shared_ptr<IConnection>>(std::string)> connectionFactory) = 0;
		virtual int getConnectionCount() = 0;

		virtual pplx::task<void> closeAllConnections(const std::string& reason) = 0;

		/// Set a timeout duration on all the current open connections
		virtual pplx::task<void> setTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

#pragma endregion
	};
}
