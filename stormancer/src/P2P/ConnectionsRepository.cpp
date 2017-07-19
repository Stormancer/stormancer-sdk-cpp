#include "stdafx.h"
#include "P2P/ConnectionsRepository.h"
#include "Logger/ILogger.h"

namespace Stormancer
{
	ConnectionsRepository::ConnectionsRepository()
	{
	}

	pplx::task<std::shared_ptr<IConnection>> Stormancer::ConnectionsRepository::addPendingConnection(const std::string& address, uint64 id)
	{
		auto tce = pplx::task_completion_event<std::shared_ptr<IConnection>>();
		PendingConnection pc;
		pc.id = id;
		pc.tce = tce;
		_pendingP2PConnections[address] = pc;
		return pplx::create_task(tce);
	}

	uint64 ConnectionsRepository::generateNewConnectionId(const std::string& address)
	{
		auto it = _pendingP2PConnections.find(address);
		if (it == _pendingP2PConnections.end())
		{
			return 0;
		}
		else
		{
			return it->second.id;
		}
	}

	void ConnectionsRepository::newConnection(std::shared_ptr<IConnection> connection)
	{
		if (connection == nullptr)
		{
			throw std::runtime_error("connection is null");
		}

		if (connection->id() != 0)
		{
			auto it = _pendingP2PConnections.find(connection->ipAddress());
			if (it != _pendingP2PConnections.end())
			{
				auto pc = it->second;
				_pendingP2PConnections.erase(it);

				_connections[connection->id()] = connection;
				ILogger::instance()->log(LogLevel::Info, "P2P", "Added pending connection , id", std::to_string(connection->id()));
				pc.tce.set(connection);
			}
			else
			{
				_connections[connection->id()] = connection;
				ILogger::instance()->log(LogLevel::Info, "P2P", "Added unknown connection, id", std::to_string(connection->id()));
			}
		}
		else
		{
			_connections[connection->id()] = connection;
		}
	}

	std::shared_ptr<IConnection> ConnectionsRepository::getConnection(uint64 id)
	{
		auto it = _connections.find(id);
		if (it == _connections.end())
		{
			ILogger::instance()->log(LogLevel::Error, "P2P", "Connection not found, id", std::to_string(id));
			return std::shared_ptr<IConnection>();
		}
		else
		{
			return std::shared_ptr<IConnection>(it->second);
		}
	}

	void ConnectionsRepository::closeConnection(std::shared_ptr<IConnection> connection, const std::string& /*reason*/)
	{
		_connections.erase(connection->id());
	}

	int ConnectionsRepository::getConnectionCount()
	{
		return (int)_connections.size();
	}

};
