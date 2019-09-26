#pragma once

#include "stormancer/BuildConfig.h"

#include "RakPeerInterface.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/StormancerTypes.h"

namespace Stormancer
{
	class P2PTunnelClient;

	class P2PTunnelRNS2EventHandler : public RakNet::RNS2EventHandler
	{
	public:
		// Inherited via RNS2EventHandler
		virtual void OnRNS2Recv(RakNet::RNS2RecvStruct * recvStruct) override;
		virtual void DeallocRNS2RecvStruct(RakNet::RNS2RecvStruct * s, const char * file, unsigned int line) override;
		virtual RakNet::RNS2RecvStruct * AllocRNS2RecvStruct(const char * file, unsigned int line) override;

	public:
		P2PTunnelClient * innerClient = nullptr;
	};

	class P2PTunnelClient
	{
		friend P2PTunnelRNS2EventHandler;

	public:

#pragma region public_methods
		
		P2PTunnelClient(std::function<void(P2PTunnelClient*, RakNet::RNS2RecvStruct*)> onMsgRecv,
			std::shared_ptr<RequestProcessor> sysCall,
			ILogger_ptr logger,
			uint16 tunnelPort,
			bool useIpv6);
		~P2PTunnelClient();

#pragma endregion

#pragma region public_members

		std::string peerSessionId;
		std::string serverId;
		RakNet::RakNetSocket2* socket;
		bool isRunning;
		byte handle;
		bool serverSide;
		unsigned short hostPort=0;

#pragma endregion

	private:

#pragma region private_methods

		void OnRNS2Recv(RakNet::RNS2RecvStruct* recvStruct);

#pragma endregion

#pragma region private_members

		std::function<void(P2PTunnelClient*, RakNet::RNS2RecvStruct*)> _onMsgRecv;
		std::shared_ptr<RequestProcessor> _sysCall;
		ILogger_ptr _logger;
		P2PTunnelRNS2EventHandler* _handler;

#pragma endregion
	};
}
