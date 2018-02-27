#include "stormancer/stdafx.h"
#include "stormancer/RakNet/RakNetConnection.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/AES/AESPacketTransform.h"

namespace Stormancer
{
	RakNetConnection::RakNetConnection(RakNet::RakNetGUID guid, int64 id, std::weak_ptr<RakNet::RakPeerInterface> peer)
		: _id(id)
		, _peer(peer)
		, _guid(guid)
		, _lastActivityDate(nowTime_t())
	{
		_connectionStateObservable.get_observable().subscribe([=](ConnectionState state) {
			// On next
			_connectionState = state;
		}, [](std::exception_ptr exptr) {
			// On error
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				ILogger::instance()->log(LogLevel::Error, "RakNetConnection", "Connection state change failed", ex.what());
			}
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

	void RakNetConnection::send(const Writer& writer, int channelUid, PacketPriority priority, PacketReliability reliability, const TransformMetadata& transformMetadata)
	{
		obytestream stream;
		std::vector<std::unique_ptr<IPacketTransform>> packetTransforms;
		packetTransforms.emplace_back(new AESPacketTransform());
		Writer writer2 = writer;
		for (auto& packetTransform : packetTransforms)
		{
			packetTransform->onSend(writer2, transformMetadata);
		}
		if (writer2)
		{
			writer2(&stream);
		}
		stream.flush();
		byte* dataPtr = stream.startPtr();
		std::streamsize dataSize = stream.writtenBytesCount();

#if defined(STORMANCER_LOG_PACKETS) || defined(STORMANCER_LOG_RAKNET_PACKETS)
		auto bytes2 = stringifyBytesArray(stream.bytes(), true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetConnection", "Send packet to scene", bytes2.c_str());
#endif

		auto peer = _peer.lock();
		if (peer)
		{
			char orderingChannel = channelUid % 16;
			auto result = peer->Send((const char*)dataPtr, (int)dataSize, priority, reliability, orderingChannel, _guid, false);
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
