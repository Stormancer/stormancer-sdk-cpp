#include "stormancer/stdafx.h"
#include "stormancer/P2P/ConnectionsRepository.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/SafeCapture.h"

namespace Stormancer
{
	ConnectionsRepository::ConnectionsRepository(ILogger_ptr logger)
		: _logger(logger)
	{
	}

	pplx::task<std::shared_ptr<IConnection>> Stormancer::ConnectionsRepository::addPendingConnection(uint64 id)
	{
		pplx::task_completion_event<std::shared_ptr<IConnection>> tce;
		{
			std::lock_guard<std::mutex> l(_mutex);
			_pendingP2PConnections.emplace(id, PendingConnection{ id,tce });
		}
		_logger->log(LogLevel::Info, "P2P", "Added pending connection from id ", std::to_string(id));
		return pplx::create_task(tce);
	}

	void ConnectionsRepository::newConnection(std::shared_ptr<IConnection> connection)
	{
		std::lock_guard<std::mutex> l(_mutex);
		_logger->log(LogLevel::Trace, "Connections", "Adding connection " + connection->ipAddress(), std::to_string(connection->id()));

		if (!connection)
		{
			throw std::runtime_error("connection is null");
		}

		auto id = connection->id();
		auto key = connection->key();

		auto wThat = STORM_WEAK_FROM_THIS();

		ConnectionContainer container;
		container.connection = connection;
		container.onCloseSubscription = connection->onClose.subscribe([wThat, id, key](std::string /*reason*/)
		{
			if (auto that = wThat.lock())
			{
				std::lock_guard<std::mutex> lg(that->_mutex);

				that->_connections.erase(id);
				that->_connectionsByKey.erase(key);
			}
		});

		auto t = pplx::task_from_result(container);
		_connections.emplace(id, t);
		_connectionsByKey.emplace(key, t);

		_logger->log(LogLevel::Info, "connections", "Completed connection", std::to_string(connection->id()));
		auto it = _pendingP2PConnections.find(connection->id());
		if (it != _pendingP2PConnections.end())
		{
			auto pc = it->second;
			_pendingP2PConnections.erase(it);
			pc.tce.set(connection);
		}
	}

	std::shared_ptr<IConnection> ConnectionsRepository::getConnection(std::string id)
	{
		std::lock_guard<std::mutex> l(_mutex);
		auto it = _connectionsByKey.find(id);
		if (it == _connectionsByKey.end())
		{
			std::string connections;
			for (auto c : _connectionsByKey)
			{
				connections += c.first;
				connections += ", ";
			}
			_logger->log(LogLevel::Debug, "connections", "Connection not found "+id+" connections : ("+connections+")","");
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
				for (auto c : _connectionsByKey)
				{
					connections += c.first;
					connections += ", ";
				}
				_logger->log(LogLevel::Debug, "connections", "Connection not complete " + id + " connections : (" + connections + ")", "");
				return std::shared_ptr<IConnection>();
			}
		}
	}

	std::shared_ptr<IConnection> ConnectionsRepository::getConnection(uint64 id)
	{
		std::lock_guard<std::mutex> l(_mutex);
		auto it = _connections.find(id);
		if (it == _connections.end())
		{
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
				return std::shared_ptr<IConnection>();
			}
		}
	}

	void ConnectionsRepository::closeConnection(std::shared_ptr<IConnection> connection, const std::string& /*reason*/)
	{
		std::lock_guard<std::mutex> l(_mutex);
		_connections.erase(connection->id());
		_connectionsByKey.erase(connection->key());
	}

	int ConnectionsRepository::getConnectionCount()
	{
		std::lock_guard<std::mutex> l(_mutex);
		return (int)_connections.size();
	}

	pplx::task<std::shared_ptr<IConnection>> ConnectionsRepository::getConnection(std::string id, std::function<pplx::task<std::shared_ptr<IConnection>>(std::string)> connectionFactory)
	{
		std::lock_guard<std::mutex> l(_mutex);
		auto it = _connectionsByKey.find(id);
		if (it != _connectionsByKey.end())
		{
			return it->second.then([](ConnectionContainer container) {return container.connection; });
		}
		else
		{
			auto t = connectionFactory(id);
			std::weak_ptr<ConnectionsRepository> wThat = this->shared_from_this();

			auto result = t.then([wThat, id](pplx::task<std::shared_ptr<IConnection>> tc) {
				try
				{
					auto connection = tc.get();
					auto pId = connection->id();
					auto key = connection->key();
					ConnectionContainer container;
					container.connection = connection;
					container.onCloseSubscription = connection->onClose.subscribe([wThat, pId, key](std::string /*reason*/) {
						if (auto that = wThat.lock())
						{
							that->_connections.erase(pId);
							that->_connectionsByKey.erase(key);
						}
					});
					if (auto that = wThat.lock())
					{
						that->_connections.emplace(pId, pplx::task_from_result(container));
					}
					return container;
				}
				catch (...)
				{
					if (auto that = wThat.lock())
					{

						that->_connectionsByKey.erase(id);
					}
					throw;
				}
			});
			_connectionsByKey.emplace(id, result);
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
			tasks.push_back(c.second.then([wThat, reason](pplx::task<ConnectionContainer> t) {
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
