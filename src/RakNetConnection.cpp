#include "stormancer.h"

namespace Stormancer
{
	RakNetConnection::RakNetConnection(RakNet::RakNetGUID guid, int64 id, RakNet::RakPeerInterface* peer, std::function<void(RakNetConnection*)> lambdaOnRequestClose)
		: _lastActivityDate(nowTime_t()),
		_guid(guid),
		_rakPeer(peer),
		_id(id)
	{
		_closeAction += lambdaOnRequestClose;
	}

	RakNetConnection::~RakNetConnection()
	{
	}

	RakNet::RakNetGUID RakNetConnection::guid()
	{
		return _guid;
	}

	time_t RakNetConnection::lastActivityDate()
	{
		return _lastActivityDate;
	}

	std::string RakNetConnection::ipAddress()
	{
		return std::string(_rakPeer->GetSystemAddressFromGuid(_guid).ToString());
	}

	bool RakNetConnection::operator==(RakNetConnection& other)
	{
		return (_id == other._id);
	}

	bool RakNetConnection::operator!=(RakNetConnection& other)
	{
		return (_id != other._id);
	}

	stringMap& RakNetConnection::metadata()
	{
		return _metadata;
	}

	void RakNetConnection::setMetadata(stringMap& metadata)
	{
		_metadata = metadata;
	}

	DependencyResolver* RakNetConnection::dependencyResolver()
	{
		return _dependencyResolver;
	}

	void RakNetConnection::close()
	{
		_closeAction(this);
	}

	int RakNetConnection::ping()
	{
		return _rakPeer->GetLastPing(_guid);
	}

	void RakNetConnection::sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		sendRaw([msgId, &writer](bytestream* stream) {
			*stream << msgId;
			writer(stream);
		}, priority, PacketReliability::RELIABLE_ORDERED, (byte)0);
	}

	void RakNetConnection::sendRaw(std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability, char channel)
	{
		bytestream stream;
		writer(&stream);
		stream.flush();
		auto bytes = stream.str();

#ifdef LOG_STORMANCER_PACKETS
		auto bytes2 = stringifyBytesArray(bytes, true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetConnection::sendToScene", "send bytes", bytes2.c_str());
#endif

		auto result = _rakPeer->Send(bytes.data(), (int)bytes.length(), priority, reliability, channel, _guid, false);
		if (result == 0)
		{
			throw std::runtime_error("Raknet failed to send the message.");
		}
	}

	void RakNetConnection::sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		bytestream stream;
		stream << sceneIndex;
		stream << route;
		writer(&stream);
		auto bytes = stream.str();

#ifdef LOG_STORMANCER_PACKETS
		auto bytes2 = stringifyBytesArray(bytes, true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetConnection::sendToScene", "send bytes", bytes2.c_str());
#endif

		auto result = _rakPeer->Send(bytes.data(), (int)bytes.length(), priority, reliability, 0, _guid, false);
		if (result == 0)
		{
			throw std::runtime_error("Raknet failed to send the message.");
		}
	}

	void RakNetConnection::setApplication(std::string account, std::string application)
	{
		if (account.length() > 0)
		{
			this->_account = account;
			this->_application = application;
		}
	}

	int64 RakNetConnection::id()
	{
		return _id;
	}

	time_t RakNetConnection::connectionDate()
	{
		return _connectionDate;
	}

	std::string RakNetConnection::account()
	{
		return _account;
	}

	std::string RakNetConnection::application()
	{
		return _application;
	}

	ConnectionState RakNetConnection::connectionState()
	{
		return _connectionState;
	}

	void RakNetConnection::setConnectionState(ConnectionState connectionState)
	{
		bool changed = (_connectionState != connectionState);
		_connectionState = connectionState;
		if (changed)
		{
			_onConnectionStateChanged(_connectionState);
		}
	}

	Action<ConnectionState>& RakNetConnection::connectionStateChangedAction()
	{
		return _onConnectionStateChanged;
	}

	Action<ConnectionState>::TIterator RakNetConnection::onConnectionStateChanged(std::function<void(ConnectionState)> callback)
	{
		return _onConnectionStateChanged.push_back(callback);
	}
};
