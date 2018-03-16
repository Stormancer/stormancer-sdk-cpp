#pragma once

#include "RakPeerInterface.h"
#include "stormancer/headers.h"
#include "stormancer/RequestProcessor.h"

namespace Stormancer
{
	class P2PTunnelClient : RakNet::RNS2EventHandler
	{
	public:

#pragma region public_methods
		
		P2PTunnelClient(std::function<void(P2PTunnelClient*, RakNet::RNS2RecvStruct*)> onMsgRecv,
			std::shared_ptr<RequestProcessor> sysCall,
			ILogger_ptr logger);
		~P2PTunnelClient();

#pragma endregion

#pragma region public_members

		uint64 peerId;
		std::string serverId;
		RakNet::RakNetSocket2* socket;
		bool isRunning;
		byte handle;
		bool serverSide;
		unsigned short hostPort;

#pragma endregion

	private:

#pragma region private_methods

		virtual void OnRNS2Recv(RakNet::RNS2RecvStruct* recvStruct) override;

		virtual void DeallocRNS2RecvStruct(RakNet::RNS2RecvStruct* s, const char* file, unsigned int line) override;

		virtual RakNet::RNS2RecvStruct* AllocRNS2RecvStruct(const char* file, unsigned int line) override;

#pragma endregion

#pragma region private_members

		std::function<void(P2PTunnelClient*, RakNet::RNS2RecvStruct*)> _onMsgRecv;
		std::shared_ptr<RequestProcessor> _sysCall;
		ILogger_ptr _logger;

#pragma endregion
	};
}
