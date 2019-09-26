#include "stormancer/stdafx.h"
#include "stormancer/P2P/ConnectionsRepository.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Utilities/PointerUtilities.h"

namespace Stormancer
{
	ConnectionsRepository::ConnectionsRepository(ILogger_ptr logger)
		: _logger(logger)
	{
	}

	pplx::task<std::shared_ptr<IConnection>> Stormancer::ConnectionsRepository::addPendingConnection(std::string clientSessionId)
	{
		_logger->log(LogLevel::Info, "connections", "Adding pending connection ( sessionId=" + clientSessionId + ")");

		std::lock_guard<std::mutex> l(_mutex);
		auto it = _pendingP2PConnections.find(clientSessionId);
		if (it == _pendingP2PConnections.end())
		{
			pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
			_pendingP2PConnections.emplace(clientSessionId, PendingConnection{ clientSessionId, tce });
			return pplx::create_task(tce);
		}
		else
		{
			return pplx::create_task(it->second.tce);
		}





	}

	void ConnectionsRepository::newConnection(std::shared_ptr<IConnection> connection)
	{
		std::lock_guard<std::mutex> l(_mutex);
		_logger->log(LogLevel::Trace, "Connections", "Adding connection (ip=" + connection->ipAddress() + ", sessionId=" + connection->sessionId() + ")");

		if (!connection)
		{
			throw std::runtime_error("connection is null");
		}

		auto id = connection->sessionId();


		auto wThat = STORM_WEAK_FROM_THIS();

		ConnectionContainer container;
		container.connection = connection;
		container.onCloseSubscription = connection->onClose.subscribe([wThat, id](std::string /*reason*/)
			{
				if (auto that = wThat.lock())
				{
					std::lock_guard<std::mutex> lg(that->_mutex);

					that->_connections.erase(id);

				}
			});

		auto t = pplx::task_from_result(container);
		_connections.emplace(id, t);


		_logger->log(LogLevel::Info, "connections", "Completed connection", connection->sessionId());
		auto it = _pendingP2PConnections.find(connection->sessionId());
		if (it != _pendingP2PConnections.end())
		{
			auto pc = it->second;
			_pendingP2PConnections.erase(it);
			connection->setSessionId(pc.sessionId);
			_logger->log(LogLevel::Info, "connections", "associated " + connection->sessionId() + " to session id" + pc.sessionId);
			pc.tce.set(connection);
		}
	}

	std::shared_ptr<IConnection> ConnectionsRepository::getConnection(std::string id)
	{
		std::lock_guard<std::mutex> l(_mutex);
		auto it = _connections.find(id);
		if (it == _connections.end())
		{
			std::string connections;
			for (auto c : _connections)
			{
				connections += c.first;
				connections += ", ";
			}
			_logger->log(LogLevel::Debug, "connections", "Connection not found (id=" + id + ", connections=" + connections + ")");
			return std::shared_ptr<IConnection>();
		}
		else
		{
			auto t = it->second;
			if (t.is_done())
			{
				return std::shared_ptr<IConnection>(t.get().connection);
			}
			else
			{
				std::string connections;
				for (auto c : _connections)
				{
					connections += c.first;
					connections += ", ";
				}
				_logger->log(LogLevel::Debug, "connections", "Connection not complete (id=" + id + ", connections=" + connections + ")");
				return std::shared_ptr<IConnection>();
			}
		}
	}



	void ConnectionsRepository::closeConnection(std::shared_ptr<IConnection> connection, const std::string& /*reason*/)
	{
		std::lock_guard<std::mutex> l(_mutex);
		_connections.erase(connection->sessionId());

	}

	int ConnectionsRepository::getConnectionCount()
	{
		std::lock_guard<std::mutex> l(_mutex);
		return (int)_connections.size();
	}

	pplx::task<std::shared_ptr<IConnection>> ConnectionsRepository::getConnection(std::string id, std::function<pplx::task<std::shared_ptr<IConnection>>(std::string)> connectionFactory)
	{
		std::lock_guard<std::mutex> l(_mutex);
		auto it = _connections.find(id);
		if (it != _connections.end())
		{
			return it->second.then([](ConnectionContainer container) {return container.connection; });
		}
		else
		{
			auto t = connectionFactory(id);
			std::weak_ptr<ConnectionsRepository> wThat = this->shared_from_this();

			auto result = t.then([wThat, id](pplx::task<std::shared_ptr<IConnection>> tc)
				{
					try
					{
						auto connection = tc.get();
						auto pId = connection->sessionId();
						auto key = connection->key();
						ConnectionContainer container;
						container.connection = connection;
						container.onCloseSubscription = connection->onClose.subscribe([wThat, pId, key](std::string /*reason*/)
							{
								if (auto that = wThat.lock())
								{
									that->_connections.erase(pId);

								}
							});
						if (auto that = wThat.lock())
						{
							
						}
						return container;
					}
					catch (...)
					{
						if (auto that = wThat.lock())
						{

						}
						throw;
					}
				});
			_connections.emplace(id,result);
			return result.then([](ConnectionContainer c) {return c.connection; });
		}
	}

	pplx::task<void> ConnectionsRepository::closeAllConnections(const std::string& reason)
	{
		std::weak_ptr<ConnectionsRepository> wThat = this->shared_from_this();
		std::vector<pplx::task<void>> tasks;
		auto connections = _connections;
		for (auto c : connections)
		{
			tasks.push_back(c.second.then([wThat, reason](pplx::task<ConnectionContainer> t)
				{
					try
					{
						t.get().connection->close(reason);
					}
					catch (...)
					{
					}
				}));

		}
		return pplx::when_all(tasks.begin(), tasks.end());
	}

	pplx::task<void> ConnectionsRepository::setTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct)
	{
		std::vector<pplx::task<void>> tasks;

		for (auto& conn : _connections)
		{
			tasks.push_back(conn.second.then([timeout, ct](ConnectionContainer connectionContainer)
				{
					return connectionContainer.connection->setTimeout(timeout, ct);
				}, ct));
		}

		return pplx::when_all(tasks.begin(), tasks.end());
	}
}
