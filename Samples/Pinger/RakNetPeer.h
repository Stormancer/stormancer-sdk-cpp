#pragma once
#include <stormancer.h>
#include "IPeer.h"

class RakNetPeer : public IPeer
{
	friend class NatService;

public:
	RakNetPeer(RakNet::RakNetGUID rakNetGUID, RakNet::RakPeerInterface* rakPeerInterface, std::string userId, bool autoConnect);
	~RakNetPeer();

	RakNetPeer(const RakNetPeer& other) = delete;
	RakNetPeer(const RakNetPeer&& other) = delete;
	RakNetPeer& operator=(const RakNetPeer& other) = delete;

public:
	void sendPacket(std::function<void(Stormancer::bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);

	/// (void*) should be casted to (RakNet::Packet*)
	void onPacketReceived(std::function<void(void*)> callback);

	void connect();
	void disconnect();
	const char* userId();
	Stormancer::ConnectionState connectionState();
	Stormancer::Action<Stormancer::ConnectionState>::TIterator onConnectionStateChanged(std::function<void(Stormancer::ConnectionState)> callback);
	Stormancer::Action<Stormancer::ConnectionState>& onConnectionStateChangedAction();
	void destroy();

	RakNet::RakNetGUID rakNetGUID() const;
	bool weAreSender() const;
	bool autoConnect() const;

	void setSystemAddress(RakNet::SystemAddress systemAddress);
	RakNet::SystemAddress systemAddress() const;

private:
	void setConnectionState(Stormancer::ConnectionState connectionState);

private:
	std::string _userId;
	RakNet::RakNetGUID _rakNetGUID;
	std::function<void(void*)> _onPacketReceived;
	Stormancer::Action<Stormancer::ConnectionState> _onConnectionStateChangedAction;
	Stormancer::ConnectionState _connectionState = Stormancer::ConnectionState::Disconnected;
	RakNet::RakPeerInterface* _rakPeerInterface = nullptr;
	bool _weAreSender = false;
	bool _autoConnect = true;
	pplx::task_completion_event<Stormancer::Result<RakNetPeer*>*> _tce;
	RakNet::SystemAddress _systemAddress;
};
