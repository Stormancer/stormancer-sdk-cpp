#include "RakNetPeer.h"

RakNetPeer::RakNetPeer(RakNet::RakNetGUID rakNetGUID, RakNet::RakPeerInterface* rakPeerInterface, std::string userId, bool autoConnect)
	: _rakNetGUID(rakNetGUID),
	_userId(userId),
	_rakPeerInterface(rakPeerInterface),
	_autoConnect(autoConnect)
{
}

RakNetPeer::~RakNetPeer()
{
}

void RakNetPeer::sendPacket(std::function<void(Stormancer::bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
{
	Stormancer::bytestream stream;
	stream << (byte)255;
	writer(&stream);
	auto data = stream.str();

#if defined(STORMANCER_LOG_PACKETS) || defined(STORMANCER_LOG_RAKNET_PACKETS)
	auto bytes2 = Stormancer::stringifyBytesArray(data, true);
	static auto guid = _rakNetGUID.ToString();
	auto msg = std::string() + "Send packet to " + guid;
	Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Trace, "RakNetConnection::sendRaw", msg.c_str(), bytes2.c_str());
#endif

	auto result = _rakPeerInterface->Send(data.c_str(), static_cast<const int>(data.length()), priority, reliability, static_cast<char>(0), _rakNetGUID, false);
}

void RakNetPeer::onPacketReceived(std::function<void(void*)> callback)
{
	_onPacketReceived = callback;
}

void RakNetPeer::connect()
{
	auto host = _systemAddress.ToString(false);
	Stormancer::uint16 port = _systemAddress.GetPort();
	_rakPeerInterface->Connect(host, port, "", 0);
}

void RakNetPeer::disconnect()
{
	setConnectionState(Stormancer::ConnectionState::Disconnecting);
	_rakPeerInterface->CloseConnection(_rakNetGUID, false);
	setConnectionState(Stormancer::ConnectionState::Disconnected);
}

const char* RakNetPeer::userId()
{
	return _userId.c_str();
}

Stormancer::ConnectionState RakNetPeer::connectionState()
{
	return _connectionState;
}

Stormancer::Action<Stormancer::ConnectionState>::TIterator RakNetPeer::onConnectionStateChanged(std::function<void(Stormancer::ConnectionState)> callback)
{
	return _onConnectionStateChangedAction.push_back(callback);
}

Stormancer::Action<Stormancer::ConnectionState>& RakNetPeer::onConnectionStateChangedAction()
{
	return _onConnectionStateChangedAction;
}

void RakNetPeer::destroy()
{
	delete this;
}

RakNet::RakNetGUID RakNetPeer::rakNetGUID() const
{
	return _rakNetGUID;
}

bool RakNetPeer::weAreSender() const
{
	return _weAreSender;
}

bool RakNetPeer::autoConnect() const
{
	return _autoConnect;
}

void RakNetPeer::setSystemAddress(RakNet::SystemAddress systemAddress)
{
	_systemAddress = systemAddress;
}

RakNet::SystemAddress RakNetPeer::systemAddress() const
{
	return _systemAddress;
}

void RakNetPeer::setConnectionState(Stormancer::ConnectionState connectionState)
{
	if (_connectionState != connectionState)
	{
		_connectionState = connectionState;
		_onConnectionStateChangedAction(_connectionState);
	}
}
