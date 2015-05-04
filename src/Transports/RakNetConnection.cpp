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

	void RakNetConnection::sendSystem(byte msgId, function<void(bytestream*)> writer)
	{
		sendRaw([msgId, &writer](bytestream* stream) {
			*stream << msgId;
			auto str1 = stream->str();
			writer(stream);
			auto str2 = stream->str();
		}, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, (uint8)0);
	}

	void RakNetConnection::sendRaw(function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability, char channel)
	{
		bytestream stream;
		writer(&stream);
		stream.flush();
		auto data = stream.str();
		auto length = data.length();
		auto result = _rakPeer->Send(data.c_str(), data.length(), (PacketPriority)priority, (PacketReliability)reliability, channel, _guid, false);
		if (result == 0)
		{
			throw exception("Failed to send message.");
		}
	}

	void RakNetConnection::sendToScene(byte sceneIndex, uint16 route, function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability)
	{
		bytestream stream;
		stream << sceneIndex;
		stream << route;
		writer(&stream);
		auto data = stream.str();
		auto result = _rakPeer->Send(data.c_str(), data.length(), priority, reliability, 0, _guid, false);

		if (result == 0)
		{
			throw exception("Failed to send message.");
		}
	}

	void RakNetConnection::setApplication(wstring account, wstring application)
	{
		if (account.length() > 0)
		{
			this->_account = account;
			this->_application = application;
		}
	}
};
