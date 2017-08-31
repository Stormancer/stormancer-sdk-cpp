#include "stdafx.h"
#include "RakNet/RakNetConnection.h"

namespace Stormancer
{
	RakNetConnection::RakNetConnection(RakNet::RakNetGUID guid, int64 id, std::weak_ptr<RakNet::RakPeerInterface> peer)
		: _lastActivityDate(nowTime_t())
		, _guid(guid)
		, _peer(peer)
		, _id(id)
	{
		_connectionStateObservable.get_observable().subscribe([this](ConnectionState state) {
			_connectionState = state;
		});
	}

	RakNetConnection::~RakNetConnection()
	{		
		close("Connection object destroyed");
	}

	RakNet::RakNetGUID RakNetConnection::guid() const
	{
		return _guid;
	}

	time_t RakNetConnection::lastActivityDate() const
	{
		return _lastActivityDate;
	}

	std::string RakNetConnection::ipAddress() const
	{
		auto rakPeer = _peer.lock();
		if (rakPeer)
		{
			return rakPeer->GetSystemAddressFromGuid(_guid).ToString(true, ':');
		}
		else
		{
			return "";
		}
	}

	bool RakNetConnection::operator==(RakNetConnection& other)
	{
		return (_id == other._id);
	}

	bool RakNetConnection::operator!=(RakNetConnection& other)
	{
		return (_id != other._id);
	}

	const std::map<std::string, std::string>& RakNetConnection::metadata() const
	{
		return _metadata;
	}

	std::string RakNetConnection::metadata(const std::string& key) const
	{
		if (mapContains(_metadata, key))
		{
			return _metadata.at(key);
		}
		return std::string();
	}

	void RakNetConnection::setMetadata(const std::map<std::string, std::string>& metadata)
	{
		_metadata = metadata;
	}

	void RakNetConnection::setMetadata(const std::string& key, const std::string& value)
	{
		_metadata[key] = value;
	}

	DependencyResolver* RakNetConnection::dependencyResolver()
	{
		return _dependencyResolver;
	}

	void RakNetConnection::close(std::string reason)
	{
		if (_connectionState == ConnectionState::Connected || _connectionState == ConnectionState::Connecting)
		{
			setConnectionState(ConnectionState::Disconnecting);
			_closeAction("");
			setConnectionState(ConnectionState::Disconnected);
		}
	}

	int RakNetConnection::ping() const
	{
		auto rakPeer = _peer.lock();
		if (rakPeer)
		{
			return rakPeer->GetLastPing(_guid);
		}
		else
		{
			return -1;
		}
	}

	void RakNetConnection::sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		sendRaw([msgId, &writer](bytestream* stream) {
			(*stream) << msgId;
			writer(stream);
		}, priority, PacketReliability::RELIABLE_ORDERED, (byte)0);
	}

	void RakNetConnection::sendRaw(std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability, char channel)
	{
		bytestream stream;
		writer(&stream);
		stream.flush();
		auto bytes = stream.str();

#if defined(STORMANCER_LOG_PACKETS) || defined(STORMANCER_LOG_RAKNET_PACKETS)
		auto bytes2 = stringifyBytesArray(bytes, true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetConnection", "Send raw packet", bytes2.c_str());
#endif

		auto peer = _peer.lock();
		if (peer)
		{
			auto result = peer->Send(bytes.data(), (int)bytes.length(), priority, reliability, channel, _guid, false);
			if (result == 0)
			{
				throw std::runtime_error("Raknet failed to send the message.");
			}
		}
		else
		{
			throw std::runtime_error("RakNet::RakPeerInterface has been destroyed.");
		}
	}

	void RakNetConnection::sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		bytestream stream;
		stream << sceneIndex;
		stream << route;
		writer(&stream);
		stream.flush();
		auto bytes = stream.str();

#if defined(STORMANCER_LOG_PACKETS) || defined(STORMANCER_LOG_RAKNET_PACKETS)
		auto bytes2 = stringifyBytesArray(bytes, true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetConnection", "Send packet to scene", bytes2.c_str());
#endif

		auto peer = _peer.lock();
		if (peer)
		{
			auto result = peer->Send(bytes.data(), (int)bytes.length(), priority, reliability, 0, _guid, false);
			if (result == 0)
			{
				throw std::runtime_error("Raknet failed to send the message.");
			}
		}
		else
		{
			throw std::runtime_error("RakNet::RakPeerInterface has been destroyed.");
		}
	}

	void RakNetConnection::setApplication(std::string account, std::string application)
	{
		if (account.length() > 0)
		{
			_account = account;
			_application = application;
		}
	}

	uint64 RakNetConnection::id() const
	{
		return _id;
	}

	time_t RakNetConnection::connectionDate() const
	{
		return _connectionDate;
	}

	const std::string& RakNetConnection::account() const
	{
		return _account;
	}

	const std::string& RakNetConnection::application() const
	{
		return _application;
	}

	ConnectionState RakNetConnection::getConnectionState() const
	{
		return _connectionState;
	}

	void RakNetConnection::setConnectionState(ConnectionState connectionState)
	{
		if (_connectionState != connectionState)
		{
			_connectionState = connectionState;
			_connectionStateObservable.get_subscriber().on_next(connectionState);

			if (connectionState == ConnectionState::Disconnected)
			{
				_connectionStateObservable.get_subscriber().on_completed();
			}
		}
	}

	void RakNetConnection::sendRaw(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		sendRaw([msgId, writer](bytestream* s) {
			*s << msgId;
			writer(s);
		}, priority, reliability,0);
	}

	rxcpp::observable<ConnectionState> RakNetConnection::getConnectionStateChangedObservable() const
	{
		return _connectionStateObservable.get_observable();
	}

	Action<std::string>::TIterator RakNetConnection::onClose(std::function<void(std::string)> callback)
	{
		return _closeAction.push_back(callback);
	}

	Action<std::string>& RakNetConnection::onCloseAction()
	{
		return _closeAction;
	}
};
