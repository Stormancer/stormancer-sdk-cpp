#include "stormancer.h"

namespace Stormancer
{
	RakNetConnection::RakNetConnection(RakNet::RakNetGUID guid, int64 id, RakNet::RakPeerInterface* peer, function<void(RakNetConnection*)> closeAction)
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

	RakNet::RakNetGUID RakNetConnection::guid()
	{
		return _guid;
	}

	time_t RakNetConnection::lastActivityDate()
	{
		return _lastActivityDate;
	}

	wstring RakNetConnection::ipAddress()
	{
		string str = _rakPeer->GetSystemAddressFromGuid(_guid).ToString();
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
		return _rakPeer->GetLastPing(_guid);
	}

	void RakNetConnection::sendSystem(byte msgId, function<void(RakNet::BitStream*)> writer)
	{
		sendRaw([msgId, &writer](RakNet::BitStream* stream) {
			stream->Write(msgId);
			writer(stream);
		}, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, (uint8)0);
	}

	void RakNetConnection::sendRaw(function<void(RakNet::BitStream*)> writer, PacketPriority priority, PacketReliability reliability, char channel)
	{
		RakNet::BitStream stream;
		writer(&stream);
		auto result = _rakPeer->Send(&stream, (PacketPriority)priority, (PacketReliability)reliability, channel, _guid, false);
		if (result == 0)
		{
			throw string("Failed to send message.");
		}
	}

	void RakNetConnection::sendToScene(byte sceneIndex, uint16 route, function<void(RakNet::BitStream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		RakNet::BitStream stream;
		stream.Write(sceneIndex);
		stream.Write(route);
		writer(&stream);
		auto result = _rakPeer->Send(&stream, priority, reliability, 0, _guid, false);

		if (result == 0)
		{
			throw string("Failed to send message.");
		}
	}
};
