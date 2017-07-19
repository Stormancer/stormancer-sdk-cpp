#pragma once
#include <stormancer.h>
#include <string>
#include "RakNetPeer.h"

class NatService : RakNet::NatPunchthroughDebugInterface
{
	friend class NatPlugin;

public:
	NatService(Stormancer::Client* client);
	~NatService();

	NatService(const NatService& other) = delete;
	NatService(const NatService&& other) = delete;
	NatService& operator=(const NatService&& other) = delete;

	pplx::task<Stormancer::Result<RakNetPeer*>*> openNat(const char* userId, bool autoConnect = true);
	pplx::task<Stormancer::Result<std::vector<RakNetPeer*>>*> openNatGroup(std::vector<std::string> userIds, bool autoConnect = true);

	void OnClientMessage(const char* msg);

	void onNewP2PConnection(std::function<void(RakNetPeer*)> callback);

	RakNet::RakNetGUID myRakNetGUID;
	RakNet::RakPeerInterface* _rakPeerInterface = nullptr;

private:
	void transportEvent(Stormancer::byte* ID, RakNet::Packet* packet, RakNet::RakPeerInterface* rakPeerInterface);
	RakNet::RakPeerInterface* getRakPeer() const;
	void disconnectPeer(RakNetPeer*);
	void upload();
	pplx::task<Stormancer::Result<Stormancer::stringMap>*> getP2PData(const char* userId);

private:
	Stormancer::Client* _client = nullptr;
	Stormancer::Scene* _scene = nullptr;
	Stormancer::IRpcService* _rpcService = nullptr;
	RakNet::NatPunchthroughClient* _natPunchthroughClient = nullptr;
	Stormancer::ITransport* _transport = nullptr;
	std::map<RakNet::RakNetGUID, RakNetPeer*> _peers;
	std::function<void(RakNetPeer*)> _onNewP2PConnection;
	pplx::task<void> _uploadTask;
};
