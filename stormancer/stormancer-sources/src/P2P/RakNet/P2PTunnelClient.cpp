#include "stormancer/stdafx.h"
#include "stormancer/P2P/RakNet/P2PTunnelClient.h"
#include "stormancer/StormancerTypes.h"

namespace Stormancer
{
	P2PTunnelClient::P2PTunnelClient(std::function<void(P2PTunnelClient*, RakNet::RNS2RecvStruct*)> onMsgRecv,
		std::shared_ptr<RequestProcessor> sysCall,
		ILogger_ptr logger,
		uint16 tunnelPort,
		bool useIpv6)
		: _sysCall(sysCall)
		, _logger(logger)
	{
		_onMsgRecv = onMsgRecv;
		socket = RakNet::RakNetSocket2Allocator::AllocRNS2();
		RakNet::RNS2_BerkleyBindParameters bbp;

		bbp.port = tunnelPort; // TODO set as param in config, only for client
		bbp.hostAddress = useIpv6 ? (char*)"::1" : (char*)"127.0.0.1";
		bbp.addressFamily = useIpv6 ? AF_INET6 : AF_INET;
		bbp.type = SOCK_DGRAM;
		bbp.protocol = 0;
		bbp.nonBlockingSocket = true;
		bbp.setBroadcast = true;
		bbp.setIPHdrIncl = false;
		bbp.doNotFragment = false;
		bbp.pollingThreadPriority = 0;

		_handler = new P2PTunnelRNS2EventHandler();
		_handler->innerClient = this;
		bbp.eventHandler = _handler;
		bbp.remotePortRakNetWasStartedOn_PS3_PS4_PSP2 = 0;
	
		RakNet::RNS2BindResult br = ((RakNet::RNS2_Berkley*)socket)->Bind(&bbp, _FILE_AND_LINE_);

		if (br == RakNet::BR_FAILED_TO_BIND_SOCKET)
		{

			RakNet::RakNetSocket2Allocator::DeallocRNS2(socket);
			throw std::runtime_error("Failed to bind socket");
		}
		else if (br == RakNet::BR_FAILED_SEND_TEST)
		{

			RakNet::RakNetSocket2Allocator::DeallocRNS2(socket);
			throw std::runtime_error("Failed to send test message");
		}
		((RakNet::RNS2_Berkley*)socket)->CreateRecvPollingThread(0).then([this](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "P2PTunnelClient", "An exception occured in raknet polling thread", ex.what());
			}
		});
		isRunning = true;
	}

	P2PTunnelClient::~P2PTunnelClient()
	{		
		if (socket != nullptr)
		{	
			auto socketCopy = socket;
			socket = nullptr;
			_handler->innerClient = nullptr;
			auto handler = _handler;
			
			pplx::create_task([socketCopy, handler]()
			{				
				((RakNet::RNS2_Berkley*)socketCopy)->BlockOnStopRecvPollingThread();
				RakNet::RakNetSocket2Allocator::DeallocRNS2(socketCopy);

				delete handler;
			});
		}
	}

	void P2PTunnelClient::OnRNS2Recv(RakNet::RNS2RecvStruct* recvStruct)
	{
		if (recvStruct->systemAddress.GetPort() != socket->GetBoundAddress().GetPort())
		{
			_onMsgRecv(this, recvStruct);
		}
	}
	

	void P2PTunnelRNS2EventHandler::OnRNS2Recv(RakNet::RNS2RecvStruct * recvStruct)
	{
		if (innerClient)
		{
			innerClient->OnRNS2Recv(recvStruct);
		}
	}

	void P2PTunnelRNS2EventHandler::DeallocRNS2RecvStruct(RakNet::RNS2RecvStruct * s, const char * file, unsigned int line)
	{
		RakNet::OP_DELETE(s, file, line);
	}

	RakNet::RNS2RecvStruct * P2PTunnelRNS2EventHandler::AllocRNS2RecvStruct(const char * file, unsigned int line)
	{
		return RakNet::OP_NEW<RakNet::RNS2RecvStruct>(file, line);
	}
}
