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
		_logger->log(LogLevel::Trace, "P2P", "Adding connection " + connection->ipAddress(), std::to_string(connection->id()));
		if (connection == nullptr)
		{
			throw std::runtime_error("connection is null");
		}

		if (connection->id() != 0)
		{
			std::lock_guard<std::mutex> l(_mutex);
			auto it = _pendingP2PConnections.find(connection->id());
			if (it != _pendingP2PConnections.end())
			{
				auto pc = it->second;
				_pendingP2PConnections.erase(it);

				auto id = connection->id();
				_connections[id] = connection;

				std::weak_ptr<ConnectionsRepository> wThat(shared_from_this());
				connection->onClose([wThat, id](std::string /*reason*/) {
					if (auto that = wThat.lock())
					{
						that->_connections.erase(id);
					}
				});

				_logger->log(LogLevel::Info, "P2P", "Transitioning connection from pending", std::to_string(connection->id()));
				pc.tce.set(connection);
			}
			else
			{
				_connections[connection->id()] = connection;
				_logger->log(LogLevel::Info, "P2P", "Added connection without pending, id", std::to_string(connection->id()));
			}
		}
		else
		{
			_connections[connection->id()] = connection;
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
			return std::shared_ptr<IConnection>(it->second);
		}
	}

	void ConnectionsRepository::closeConnection(std::shared_ptr<IConnection> connection, const std::string& /*reason*/)
	{
		std::lock_guard<std::mutex> l(_mutex);
		_connections.erase(connection->id());
	}

	int ConnectionsRepository::getConnectionCount()
	{
		std::lock_guard<std::mutex> l(_mutex);
		return (int)_connections.size();
	}
};
