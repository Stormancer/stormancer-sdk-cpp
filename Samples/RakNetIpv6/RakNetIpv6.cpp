// testIpv6.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "stormancer.h"
#include "RakPeer.h"
#include <iostream>
#include <thread>

RakNet::RakPeerInterface* StartRaknet(unsigned short port, bool enableIpv4, bool enableIpv6)
{
	auto rakPeer = RakNet::RakPeerInterface::GetInstance();

	DataStructures::List<RakNet::SocketDescriptor> socketDescriptors;	
	int nb = 0;
	if (enableIpv6)
	{
		nb++;
		RakNet::SocketDescriptor socketDescriptorIpv6(port,"");
		socketDescriptorIpv6.port = port;
		
		socketDescriptorIpv6.socketFamily = AF_INET6;

		socketDescriptors.Push(socketDescriptorIpv6, _FILE_AND_LINE_);
	}
	if (enableIpv4)
	{
		nb++;
		RakNet::SocketDescriptor socketDescriptorIpv4(port,"");
		socketDescriptorIpv4.port = port;
		
		socketDescriptorIpv4.socketFamily = AF_INET;

		socketDescriptors.Push(socketDescriptorIpv4, _FILE_AND_LINE_);
	}
	rakPeer->SetMaximumIncomingConnections(100);
	auto result = rakPeer->Startup(10, socketDescriptors);

	return rakPeer;
}

int main(int argc, const char* argv[])
{
	auto client = argc > 1;
	RakNet::RakPeerInterface* peer;
	bool ipv6 = false, ipv4 = false;
	for (int i = 2; i < argc; i++)
	{
		if (std::string(argv[i]) == "ipv4")
		{
			ipv4 = true;
		}
		if (std::string(argv[i]) == "ipv6")
		{
			ipv6 = true;
		}
	}

	if (!client)
	{
		peer = StartRaknet(7777, true, true);
	}
	else
	{
		peer = StartRaknet(0, ipv4, ipv6);
	}

	if (client)
	{
		auto host = std::string(argv[1]);
		//std::cout << "connecting to " << host << std::endl;
		auto result = peer->Connect(host.c_str(), 7777, nullptr, 0);
		//std::cout << result << std::endl;
	}


	RakNet::Packet* p = nullptr;
	while (true)
	{
		while ((p = peer->Receive()) != nullptr)
		{
			std::cout << " Received packet " << std::to_string(p->data[0]) << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}




	return 0;
}


