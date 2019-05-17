#include "stormancer/stdafx.h"
#include "stormancer/P2P/RelayConnection.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	RelayConnection::RelayConnection(std::shared_ptr<IConnection> serverConnection, std::string address, uint64 remotePeerId, std::string p2pSessionId, std::weak_ptr<Serializer> serializer)
		: _serverConnection(serverConnection)
		, _p2pSessionId(p2pSessionId)
		, _remotePeerId(remotePeerId)
		, _ipAddress(address)
		, _serializer(serializer)
		, _dependencyResolver(std::make_shared<DependencyResolver>(serverConnection->dependencyResolver()))
	{
	}

	void RelayConnection::send(const StreamWriter& streamWriter, int channelUid, PacketPriority priority, PacketReliability reliability, const TransformMetadata& /*transformMetadata*/)
	{
		auto wSerializer = _serializer;
		auto p2pSessionId = _p2pSessionId;
		_serverConnection->send([wSerializer, p2pSessionId, priority, reliability, streamWriter](obytestream& stream)
		{
			stream << (byte)MessageIDTypes::ID_P2P_RELAY;
			if (auto serializer = wSerializer.lock())
			{
				serializer->serialize(stream, p2pSessionId);
				serializer->serialize(stream, (uint8)priority);
				serializer->serialize(stream, (uint8)reliability);

				if (streamWriter)
				{
					streamWriter(stream);
				}
			}
		}, channelUid, priority, reliability);
	}

	void RelayConnection::setApplication(std::string account, std::string application)
	{
		throw std::runtime_error("Not implemented");
	}

	void RelayConnection::close(std::string reason)
	{
	}

	std::string RelayConnection::ipAddress() const
	{
		return _ipAddress;
	}

	int RelayConnection::ping() const
	{
		return _serverConnection->ping();
	}

	std::string RelayConnection::key() const
	{
		return _p2pSessionId;
	}

	uint64 RelayConnection::id() const
	{
		return _remotePeerId;
	}

	time_t RelayConnection::connectionDate() const
	{
		return _connectionDate;
	}

	const std::string & RelayConnection::account() const
	{
		return _serverConnection->account();
	}

	const std::string & RelayConnection::application() const
	{
		return _serverConnection->application();
	}

	const std::map<std::string, std::string>& RelayConnection::metadata() const
	{
		return _metadata;
	}

	std::string RelayConnection::metadata(const std::string & key) const
	{
		auto it = _metadata.find(key);
		if (it != _metadata.end())
		{
			return (*it).second;
		}
		else
		{
			return std::string();
		}
	}

	void RelayConnection::setMetadata(const std::map<std::string, std::string>& metadata)
	{
		_metadata = metadata;
	}

	void RelayConnection::setMetadata(const std::string & key, const std::string & value)
	{
		_metadata[key] = value;
	}

	std::shared_ptr<DependencyResolver> RelayConnection::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	ConnectionState RelayConnection::getConnectionState() const
	{
		return _serverConnection->getConnectionState();
	}

	rxcpp::observable<ConnectionState> RelayConnection::getConnectionStateChangedObservable() const
	{
		return _serverConnection->getConnectionStateChangedObservable();
	}

	void RelayConnection::setConnectionState(ConnectionState)
	{
		throw std::runtime_error("Not supported");
	}

	

	
}
