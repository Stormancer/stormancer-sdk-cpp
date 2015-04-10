#include "stormancer.h"

namespace Stormancer
{
	RakNetConnection::RakNetConnection(RakNetGUID* guid, int64 id, RakPeerInterface* peer, function<void(RakNetConnection*)> closeAction)
		: _lastActivityDate(Helpers::nowTime_t()),
		_guid(guid),
		_rakPeer(peer),
		_closeAction(closeAction)
	{
		IConnection::_id = id;
	}

	RakNetConnection::~RakNetConnection()
	{
	}

	RakNetGUID* RakNetConnection::guid()
	{
		return _guid;
	}

	time_t RakNetConnection::lastActivityDate()
	{
		return _lastActivityDate;
	}

	wstring RakNetConnection::ipAddress()
	{
		string str = _rakPeer->GetSystemAddressFromGuid(*_guid).ToString();
		return Helpers::to_wstring(str);
	}

	bool RakNetConnection::operator==(RakNetConnection& other)
	{
		return (this->_id == other._id);
	}

	bool RakNetConnection::operator!=(RakNetConnection& other)
	{
		return (this->_id != other._id);
	}

	stringMap RakNetConnection::metadata()
	{
		return _metadata;
	}

	void RakNetConnection::close()
	{
		_closeAction(this);
	}

	int RakNetConnection::ping()
	{
		return _rakPeer->GetLastPing(*_guid);
	}

	void RakNetConnection::sendSystem(byte msgId, function<void(byteStream*)> writer)
	{
		// TODO
	}

	void RakNetConnection::sendRaw(function<void(byteStream*)> writer, PacketPriority priority, PacketReliability reliability, uint8 channel)
	{
		// TODO
	}

	void RakNetConnection::sendToScene(byte sceneIndex, uint16 route, function<void(byteStream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		// TODO
	}
};
