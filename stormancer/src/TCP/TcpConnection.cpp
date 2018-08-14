#include "stormancer/stdafx.h"
#include "stormancer/TCP/TcpConnection.h"

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

	std::weak_ptr<DependencyResolver> TcpConnection::dependencyResolver()
	{
		return std::weak_ptr<DependencyResolver>();
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

	Action<std::string>& TcpConnection::onCloseAction()
	{
		return _closeAction;
	}

	void TcpConnection::send(const Writer& writer, int /*channelUid*/, PacketPriority /*priority*/, PacketReliability /*reliability*/, const TransformMetadata& /*transformMetadata*/)
	{
		if (!_socketId.expired())
		{
			//SOCKET socketId = *(_socketId.lock());
			obytestream stream;
			if (writer)
			{
				writer(&stream);
			}
			stream.flush();
			auto bytes = stream.bytes();

			//unsigned int length = (unsigned int)bytes.length();
			//send__(socketId, (char*)&length, sizeof(length), 0);
			//send__(socketId, bytes.data(), length, 0);
		}
	}
}
