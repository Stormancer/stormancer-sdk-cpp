#include "stdafx.h"
#include "TCP/TcpConnection.h"

namespace Stormancer
{
	TcpConnection::TcpConnection(std::shared_ptr<SOCKET> socketId, uint64 connectionId, std::string ip)
		:_socketId(socketId)
		, _id(connectionId)
		, _ip(ip)
	{
	}

	TcpConnection::~TcpConnection()
	{
	}

	void TcpConnection::sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		sendRaw([msgId, &writer](bytestream* stream) {
			*stream << msgId;
			writer(stream);
		}, priority, PacketReliability::RELIABLE_ORDERED);
	}

	void TcpConnection::sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		sendRaw([sceneIndex, route, &writer](bytestream* stream) {
			*stream << sceneIndex;
			*stream << route;
			writer(stream);
		}, priority, reliability);
	}

	void TcpConnection::setApplication(std::string account, std::string application)
	{
		if (account.length() > 0)
		{
			_account = account;
			_application = application;
		}
	}

	void TcpConnection::close(std::string reason)
	{
		if (_connectionState != ConnectionState::Disconnected && _connectionState != ConnectionState::Disconnecting)
		{
			setConnectionState(ConnectionState::Disconnecting);
			_closeAction("");
			setConnectionState(ConnectionState::Disconnected);
		}
	}

	std::string TcpConnection::ipAddress() const
	{
		return _ip;
	}

	int TcpConnection::ping() const
	{
		return 0;
	}

	uint64 TcpConnection::id() const
	{
		return _id;
	}

	time_t TcpConnection::connectionDate() const
	{
		return _connectionDate;
	}

	const std::string& TcpConnection::account() const
	{
		return _account;
	}

	const std::string& TcpConnection::application() const
	{
		return _application;
	}

	const std::map<std::string, std::string>& TcpConnection::metadata() const
	{
		return _metadata;
	}

	std::string TcpConnection::metadata(const std::string& key) const
	{
		if (mapContains(_metadata, key))
		{
			return _metadata.at(key);
		}
		return std::string();
	}

	void TcpConnection::setMetadata(const std::map<std::string, std::string>& metadata)
	{
		_metadata = metadata;
	}

	void TcpConnection::setMetadata(const std::string& key, const std::string& value)
	{
		_metadata[key] = value;
	}

	DependencyResolver* TcpConnection::dependencyResolver()
	{
		return nullptr;
	}

	ConnectionState TcpConnection::getConnectionState() const
	{
		return _connectionState;
	}

	rxcpp::observable<ConnectionState> TcpConnection::getConnectionStateChangedObservable() const
	{
		return _connectionStateObservable.get_observable();
	}

	void TcpConnection::setConnectionState(ConnectionState connectionState)
	{
		_connectionState = connectionState;
		_connectionStateObservable.get_subscriber().on_next(connectionState);
	}

	Action<std::string>::TIterator TcpConnection::onClose(std::function<void(std::string)> callback)
	{
		return _closeAction.push_back(callback);
	}

	void TcpConnection::sendRaw(std::function<void(bytestream*)> writer, PacketPriority, PacketReliability)
	{
		if (!_socketId.expired())
		{
			SOCKET socketId = *(_socketId.lock());
			bytestream stream;
			writer(&stream);
			stream.flush();
			auto bytes = stream.str();

			unsigned int length = (unsigned int)bytes.length();
			send__(socketId, (char*)&length, sizeof(length), 0);
			send__(socketId, bytes.data(), length, 0);
		}
	}
	
	void TcpConnection::sendRaw(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		sendRaw([msgId, writer](bytestream* s) {
			*s << msgId;
			writer(s);
		}, priority, reliability);
	}
}
