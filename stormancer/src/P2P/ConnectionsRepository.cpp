#include "stormancer/stdafx.h"
#include "stormancer/P2P/ConnectionsRepository.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	ConnectionsRepository::ConnectionsRepository()
	{
	}

	pplx::task<std::shared_ptr<IConnection>> Stormancer::ConnectionsRepository::addPendingConnection( uint64 id)
	{
		auto tce = pplx::task_completion_event<std::shared_ptr<IConnection>>();
		PendingConnection pc;
		pc.id = id;
		pc.tce = tce;
		_pendingP2PConnections[id] = pc;
		ILogger::instance()->log(LogLevel::Info, "P2P", "Added pending connection from id ", std::to_string(id));
		return pplx::create_task(tce);
	}

	void ConnectionsRepository::newConnection(std::shared_ptr<IConnection> connection)
	{
		ILogger::instance()->log(LogLevel::Trace, "P2P", "Adding connection " + connection->ipAddress(), std::to_string(connection->id()));
		if (connection == nullptr)
		{
			throw std::runtime_error("connection is null");
		}

		if (connection->id() != 0)
		{
			auto it = _pendingP2PConnections.find(connection->id());
			if (it != _pendingP2PConnections.end())
			{
				auto pc = it->second;
				_pendingP2PConnections.erase(it);

				auto id = connection->id();
				_connections[id] = connection;
				
				std::weak_ptr<ConnectionsRepository> weakThis(shared_from_this());
				connection->onClose([=](std::string reason) {
					if (auto thiz = weakThis.lock())
					{
						thiz->_connections.erase(id);
					}
				});

				ILogger::instance()->log(LogLevel::Info, "P2P", "Transitioning connection from pending", std::to_string(connection->id()));
				pc.tce.set(connection);
			}
			else
			{
				_connections[connection->id()] = connection;
				ILogger::instance()->log(LogLevel::Info, "P2P", "Added connection without pending, id", std::to_string(connection->id()));
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
