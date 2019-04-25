#include "stormancer/stdafx.h"
#include "stormancer/RakNet/RakNetConnection.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/AES/AESPacketTransform.h"
#include "stormancer/AES/IAES.h"
#include "stormancer/Configuration.h"
#include "stormancer/ChannelUidStore.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/Helpers.h"
#include "stormancer/Exceptions.h"
#include <limits>
#include <chrono>

#undef max
#undef min

namespace Stormancer
{
	RakNetConnection::RakNetConnection(RakNet::RakNetGUID guid, int64 id,std::string key, std::weak_ptr<RakNet::RakPeerInterface> peer, ILogger_ptr logger, std::shared_ptr<DependencyResolver> resolver)
		: _id(id)
		, _key(key)
		, _peer(peer)
		, _guid(guid)
		, _lastActivityDate(nowTime_t())
		, _dependencyResolver(resolver)
		, _logger(logger)
	{
		auto onNext = [=](ConnectionState state) {
			_connectionState = state;
		};

		auto onError = [=](std::exception_ptr exptr) {
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "RakNetConnection", "Connection state change failed", ex.what());
			}
		};
		
		_connectionStateObservable.get_observable().subscribe(onNext, onError);

		_dependencyResolver->registerDependency<ChannelUidStore>([](auto dr) {
			return std::make_shared<ChannelUidStore>();
		},true);
	}

	RakNetConnection::~RakNetConnection()
	{
		_logger->log(LogLevel::Trace, "raknetConnection", "Destroying connection object ", std::to_string(id()));
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

	pplx::task<void> RakNetConnection::updatePeerMetadata(pplx::cancellation_token ct)
	{
		auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();
		auto logger = _logger;

		ct = create_linked_shutdown_token(ct);

		return requestProcessor->sendSystemRequest(this, (byte)SystemRequestIDTypes::ID_SET_METADATA, [this](obytestream& stream)
		{
			_dependencyResolver->resolve<Serializer>()->serialize(stream, metadata());
		}, PacketPriority::MEDIUM_PRIORITY, ct)
			.then([logger](Packet_ptr)
		{
			logger->log(LogLevel::Trace, "RakNetConnection::updatePeerMetadata", "Updated peer metadata");
		}, ct);
	}

	std::shared_ptr<DependencyResolver> RakNetConnection::dependencyResolver()
	{
		return _dependencyResolver;
	}

	void RakNetConnection::close(std::string reason)
	{
		_logger->log(LogLevel::Trace, "raknetConnection", "Closing connection " + reason, std::to_string(id()));
		if (_connectionState == ConnectionState::Connected || _connectionState == ConnectionState::Connecting)
		{
			setConnectionState(ConnectionState(ConnectionState::Disconnecting, reason));
			onClose(reason);
			setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
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

	void RakNetConnection::send(const StreamWriter& streamWriter, int channelUid, PacketPriority priority, PacketReliability reliability, const TransformMetadata& transformMetadata)
	{
		auto dependencyResolver = _dependencyResolver;
		if (!dependencyResolver)
		{
			throw std::runtime_error("Dependency resolver not available");
		}

		obytestream stream;
		std::vector<std::shared_ptr<IPacketTransform>> packetTransforms;
		// TODO handle encryption for p2p connections
		if (metadata("type") != "p2p")
		{
			packetTransforms.emplace_back(std::make_shared<AESPacketTransform>(dependencyResolver->resolve<IAES>(), dependencyResolver->resolve<Configuration>()));
		}
		StreamWriter writer2 = streamWriter;
		for (auto& packetTransform : packetTransforms)
		{
			packetTransform->onSend(writer2, this->id(), transformMetadata);
		}
		if (writer2)
		{
			writer2(stream);
		}
		stream.flush();
		byte* dataPtr = stream.startPtr();
		std::streamsize dataSize = stream.currentPosition();

#if defined(STORMANCER_LOG_PACKETS) || defined(STORMANCER_LOG_RAKNET_PACKETS)
		auto bytes2 = stringifyBytesArray(stream.bytes(), true, true);
		_logger->log(LogLevel::Trace, "RakNetConnection", "Send packet to scene", bytes2.c_str());
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

	std::string RakNetConnection::key() const
	{
		return _key;
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

	pplx::task<void> RakNetConnection::setTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct)
	{
		auto totalMs = timeout.count();
		auto totalMsStr = std::to_string(totalMs);
		if (totalMs > static_cast<std::chrono::milliseconds::rep>(std::numeric_limits<RakNet::TimeMS>::max()) ||
			totalMs < static_cast<std::chrono::milliseconds::rep>(std::numeric_limits<RakNet::TimeMS>::min()))
		{
			_logger->log(LogLevel::Error, "RakNetConnection::setTimeout", "timeout value doesn't fit in RakNet::TimeMS", totalMsStr);
			return pplx::task_from_exception<void>(std::out_of_range("timeout value doesn't fit in RakNet::TimeMS"));
		}
		
		if (_connectionState != ConnectionState::Connected)
		{
			_logger->log(LogLevel::Error, "RakNetConnection::setTimeout", "Failed to set timeout because connection is not established", std::to_string(_connectionState));
			return pplx::task_from_result();// pplx::task_from_exception<void>(std::runtime_error("cannot set timeout when not in Connected state"));
		}

		setMetadata("timeout", totalMsStr);
		if (auto peer = _peer.lock())
		{
			peer->SetTimeoutTime(static_cast<RakNet::TimeMS>(totalMs), peer->GetSystemAddressFromGuid(_guid));
		}
		else
		{
			return pplx::task_from_exception<void>(PointerDeletedException("peer"));
		}
		return updatePeerMetadata(ct);
	}
};
