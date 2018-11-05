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
		auto t = pplx::task_from_result(connection);
		_connections.emplace(id, t);
		_connectionsByKey.emplace(key, t);

		std::weak_ptr<ConnectionsRepository> wThat(shared_from_this());
		connection->onClose([wThat, id, key](std::string /*reason*/) {
			if (auto that = wThat.lock())
			{
				that->_connections.erase(id);
				that->_connectionsByKey.erase(key);
			}
		});

		_logger->log(LogLevel::Info, "P2P", "Transitioning connection from pending", std::to_string(connection->id()));
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
			return std::shared_ptr<IConnection>();
		}
		else
		{
			auto t = it->second;
			if (t.is_done())
			{
				return std::shared_ptr<IConnection>(t.get());
			}
			else
			{
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
				return std::shared_ptr<IConnection>(t.get());
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
			return it->second;
		}
		else
		{
			auto t = connectionFactory(id);
			std::weak_ptr<ConnectionsRepository> wThat = this->shared_from_this();

			auto result= t.then([wThat,id](pplx::task<std::shared_ptr<IConnection>> tc) {
				try
				{
					auto connection = tc.get();
					auto pId = connection->id();
					auto key = connection->key();

					connection->onClose([wThat, pId, key](std::string /*reason*/) {
						if (auto that = wThat.lock())
						{
							that->_connections.erase(pId);
							that->_connectionsByKey.erase(key);
						}
						
					});
					if (auto that = wThat.lock())
					{
						that->_connections.emplace(pId, pplx::task_from_result(connection));
					}
					return connection;
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
			return result;
		}

		
	}
	pplx::task<void> ConnectionsRepository::closeAllConnections(const std::string& reason)
	{
		std::weak_ptr<ConnectionsRepository> wThat = this->shared_from_this();
		std::vector<pplx::task<void>> tasks;
		auto connections = _connections;
		for (auto c : connections)
		{
			tasks.push_back(c.second.then([wThat,reason](pplx::task<std::shared_ptr<IConnection>> t) {
				try
				{
					t.get()->close(reason);
				}
				catch (...)
				{

				}
			}));
			
		}
		return pplx::when_all(tasks.begin(), tasks.end());
			
	}
};
