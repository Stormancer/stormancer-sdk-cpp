#pragma once

#include "headers.h"
#include "RequestProcessor.h"
#include "Serializer.h"
#include "IConnectionManager.h"
#include "P2P/P2PTunnel.h"
#include "P2P/ServerDescriptor.h"
#include "Configuration.h"
namespace RakNet
{
	struct RNS2RecvStruct;
}

namespace Stormancer
{
	using peerHandle = std::tuple<uint64, byte>;

	class P2PTunnelClient;

	struct PeerHandle_hash : public std::unary_function<peerHandle, std::size_t>
	{
		std::size_t operator()(const peerHandle& k) const;
	};

	struct PeerHandle_equal : public std::binary_function<peerHandle,peerHandle, bool>
	{
		bool operator()(const peerHandle& v0, const peerHandle& v1) const;
	};

	class P2PTunnels
	{
	public:

#pragma region public_methods

		P2PTunnels(std::shared_ptr<RequestProcessor> sysCall, std::shared_ptr<IConnectionManager> connections, std::shared_ptr<Serializer> serializer, std::shared_ptr<Configuration> configuration);
		void receiveFrom(uint64 id, bytestream* stream);
		std::shared_ptr<P2PTunnel> createServer(std::string serverId, std::shared_ptr<P2PTunnels> tunnels);
		pplx::task<std::shared_ptr<P2PTunnel>> openTunnel(uint64 connectionId, std::string serverId);
		byte addClient(std::string serverId, uint64 clientPeerId);
		void closeTunnel(byte handle, uint64 peerId);
		pplx::task<void> destroyServer(std::string serverId);
		pplx::task<void> destroyTunnel(uint64 peerId, byte handle);
		void onMsgReceived(P2PTunnelClient* client, RakNet::RNS2RecvStruct* recvStruct);

#pragma endregion

	private:

#pragma region private_members

		std::shared_ptr<RequestProcessor> _sysClient;
		std::shared_ptr<IConnectionManager> _connections;
		std::shared_ptr<Serializer> _serializer;
		std::shared_ptr<Configuration> _config;
		std::unordered_map<std::string, ServerDescriptor> _servers;
		//Tunnel targeting the key [remotePeerId,handle]
		std::unordered_map < peerHandle, std::shared_ptr<P2PTunnelClient>,  PeerHandle_hash, PeerHandle_equal> _tunnels;

#pragma endregion
	};
}
