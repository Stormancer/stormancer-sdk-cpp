#include "stdafx.h"
#include "P2P/RakNet/P2PTunnelClient.h"

namespace Stormancer
{
	P2PTunnelClient::P2PTunnelClient(std::function<void(P2PTunnelClient*, RakNet::RNS2RecvStruct*)> onMsgRecv, std::shared_ptr<RequestProcessor> sysCall)
		: _sysCall(sysCall)
	{
		_onMsgRecv = onMsgRecv;
		hostPort = 0;
		socket = RakNet::RakNetSocket2Allocator::AllocRNS2();
		RakNet::RNS2_BerkleyBindParameters bbp;

		bbp.port = 0;
		bbp.hostAddress = "127.0.0.1";
		bbp.addressFamily = AF_INET;
		bbp.type = SOCK_DGRAM;
		bbp.protocol = 0;
		bbp.nonBlockingSocket = true;
		bbp.setBroadcast = true;
		bbp.setIPHdrIncl = false;
		bbp.doNotFragment = false;
		bbp.pollingThreadPriority = 0;
		bbp.eventHandler = this;
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
		((RakNet::RNS2_Berkley*)socket)->CreateRecvPollingThread(0).then([](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (const std::exception& ex)
			{
				ILogger::instance()->log(LogLevel::Error, "P2PTunnelClient", "An exception occured in raknet polling thread", ex.what());
			}
		});
		isRunning = true;
	}

	P2PTunnelClient::~P2PTunnelClient()
	{
		if (socket != nullptr)
		{
			((RakNet::RNS2_Berkley*)socket)->BlockOnStopRecvPollingThread();
			RakNet::RakNetSocket2Allocator::DeallocRNS2(socket);
			socket = nullptr;
		}
	}

	void P2PTunnelClient::OnRNS2Recv(RakNet::RNS2RecvStruct* recvStruct)
	{
		if (recvStruct->systemAddress.GetPort() != socket->GetBoundAddress().GetPort())
		{
			_onMsgRecv(this, recvStruct);
		}
	}

	void P2PTunnelClient::DeallocRNS2RecvStruct(RakNet::RNS2RecvStruct* s, const char * file, unsigned int line)
	{
		RakNet::OP_DELETE(s, file, line);
	}

	RakNet::RNS2RecvStruct* P2PTunnelClient::AllocRNS2RecvStruct(const char * file, unsigned int line)
	{
		return RakNet::OP_NEW<RakNet::RNS2RecvStruct>(file, line);
	}
};
