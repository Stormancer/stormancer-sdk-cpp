
#include "testP2P.h"

#include "RakNetSocket2.h"
#include "RakPeer.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Logger/ConsoleLogger.h"
#include "stormancer/IClient.h"
#include "stormancer/RPC/Service.h"

//static std::string endpoint = "http://api.stormancer.com:8081/";
//static std::string account = "samples";
//static std::string application = "p2p";
//static std::string scene = "test-scene";


class UdpSocket : RakNet::RNS2EventHandler
{
public:

	UdpSocket(std::shared_ptr<Stormancer::ILogger> logger, unsigned short port, bool server) :
		_logger(logger),
		_server(server)
	{
		socket = RakNet::RakNetSocket2Allocator::AllocRNS2();
		RakNet::RNS2_BerkleyBindParameters bbp;

		bbp.port = port;
		bbp.hostAddress = (char*)"UNASSIGNED_SYSTEM_ADDRESS";
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

		((RakNet::RNS2_Berkley*)socket)->CreateRecvPollingThread(0);
	}

	~UdpSocket()
	{
		if (socket != nullptr)
		{
			((RakNet::RNS2_Berkley*)socket)->BlockOnStopRecvPollingThread();
			RakNet::RakNetSocket2Allocator::DeallocRNS2(socket);
		}
	}

	void Send(unsigned short port, const std::string& data = "blah")
	{
		RakNet::RNS2_SendParameters sp;
		sp.data = (char*)data.c_str();
		sp.length = 5;
		sp.systemAddress.FromStringExplicitPort("127.0.0.1", port);
		socket->Send(&sp, _FILE_AND_LINE_);
	}

	// Hérité via RNS2EventHandler
	virtual void OnRNS2Recv(RakNet::RNS2RecvStruct * recvStruct) override
	{
		if (recvStruct->systemAddress.GetPort() != socket->GetBoundAddress().GetPort())
		{
			auto data = std::string(recvStruct->data);
			_logger->log("P2P sample: received a message: " + data);
			if (_server)
			{
				Send(recvStruct->systemAddress.GetPort(), data);
			}
		}
	}

	virtual void DeallocRNS2RecvStruct(RakNet::RNS2RecvStruct * s, const char * file, unsigned int line) override
	{
		RakNet::OP_DELETE(s, file, line);
	}

	virtual RakNet::RNS2RecvStruct * AllocRNS2RecvStruct(const char * file, unsigned int line) override
	{
		return RakNet::OP_NEW<RakNet::RNS2RecvStruct>(file, line);
	}

private:

	std::shared_ptr<Stormancer::ILogger> _logger;

	RakNet::RakNetSocket2* socket;

	bool _server;
};

void p2pConnect(std::shared_ptr<Stormancer::IClient> host, std::shared_ptr<Stormancer::IClient> guest, Stormancer::ILogger_ptr logger, std::string sceneId);

void testP2P(std::string endpoint, std::string account, std::string application, std::string sceneId)
{
	auto logger = std::make_shared<Stormancer::ConsoleLogger>();

	auto hostConfiguration = Stormancer::Configuration::create(endpoint, account, application);
	hostConfiguration->logger = logger;
	//hostConfiguration->encryptionEnabled = true;

	auto hostClient = Stormancer::IClient::create(hostConfiguration);

	auto guestConfiguration = Stormancer::Configuration::create(endpoint, account, application);
	guestConfiguration->logger = logger;
	guestConfiguration->serverGamePort = 7778;
	//guestConfiguration->encryptionEnabled = true;

	auto guestClient = Stormancer::IClient::create(guestConfiguration);

	try
	{
		p2pConnect(hostClient, guestClient, logger, sceneId);
		p2pConnect(guestClient, hostClient, logger, sceneId);
	}
	catch (const std::exception& e)
	{
		logger->log(e);
	}
}

Stormancer::Subscription hostPeerConnectedSub, guestPeerConnectedSub;

void p2pConnect(std::shared_ptr<Stormancer::IClient> hostClient, std::shared_ptr<Stormancer::IClient> guestClient, Stormancer::ILogger_ptr logger, std::string sceneId)
{
	std::shared_ptr<Stormancer::Scene> hostScene;
	std::shared_ptr<Stormancer::Scene> guestScene;
	pplx::task_completion_event<std::shared_ptr<Stormancer::IP2PScenePeer>> hostPeerTce;

	try
	{
		hostScene = hostClient->connectToPublicScene(sceneId, [logger](std::shared_ptr<Stormancer::Scene> scene)
		{
			scene->addRoute("test", [logger](Stormancer::Packetisp_ptr packet)
			{
				logger->log(Stormancer::LogLevel::Info, "P2P_test", "host received a scene message from a peer", packet->readObject<std::string>());
			}, Stormancer::MessageOriginFilter::Peer);
		}).get();
	}
	catch (const std::exception& ex)
	{
		logger->log(ex);
		throw;
	}

	try
	{
		guestScene = guestClient->connectToPublicScene(sceneId, [logger](std::shared_ptr<Stormancer::Scene> scene)
		{
			scene->addRoute("test", [logger](Stormancer::Packetisp_ptr packet)
			{
				logger->log(Stormancer::LogLevel::Info, "P2P_test", "guest received a scene message from a peer", packet->readObject<std::string>());
			}, Stormancer::MessageOriginFilter::Peer);
		}).get();
	}
	catch (const std::exception& ex)
	{
		logger->log(ex);
		throw;
	}

	hostPeerConnectedSub = hostScene->onPeerConnected().subscribe([logger, hostPeerTce](std::shared_ptr<Stormancer::IP2PScenePeer> peer)
	{
		logger->log(Stormancer::LogLevel::Info, "P2P_test", "Peer connected on host scene", peer->sessionId());
		hostPeerTce.set(peer);
	});

	guestPeerConnectedSub = guestScene->onPeerConnected().subscribe([logger](std::shared_ptr<Stormancer::IP2PScenePeer> peer)
	{
		logger->log(Stormancer::LogLevel::Info, "P2P_test", "Peer connected on guest scene", peer->sessionId());
	});

	auto hostRpc = hostScene->dependencyResolver().resolve<Stormancer::RpcService>();
	auto guestRpc = guestScene->dependencyResolver().resolve<Stormancer::RpcService>();
	std::string p2pToken = guestRpc->rpc<std::string>("getP2PToken", true).get();



	logger->log(std::string("Got P2P token ") + p2pToken);

	auto hostTunnel = hostScene->registerP2PServer("pingServer");
	UdpSocket hostSocket(logger, hostTunnel->port, true);

	logger->log(std::string("Retrieved host tunnel: ") + hostTunnel->ip + ":" + std::to_string(hostTunnel->port));

	auto guestPeer = guestScene->openP2PConnection(p2pToken).get();
	logger->log("Retrieved guestPeer.");

	auto hostPeer = pplx::create_task(hostPeerTce).get();
	guestPeer->send("test", [](Stormancer::obytestream& stream) {
		Stormancer::Serializer{}.serialize(stream, std::string("message from guest!"));
	});
	hostPeer->send("test", [](Stormancer::obytestream& stream) {
		Stormancer::Serializer{}.serialize(stream, std::string("message from host!"));
	});

	// Test comparison operators
	assert(guestPeer != hostPeer);
	assert(!(guestPeer == hostPeer));

	auto guestTunnel = guestPeer->openP2PTunnel("pingServer").get();
	logger->log(std::string("Retrieved guest tunnel: ") + guestTunnel->ip + ":" + std::to_string(guestTunnel->port));
	UdpSocket guestSocket(logger, 0, false);
	guestSocket.Send(guestTunnel->port);

	auto reverseGuestTunnel = guestScene->registerP2PServer("pongServer");
	UdpSocket reverseGuestSocket(logger, reverseGuestTunnel->port, true);
	logger->log(std::string("Retrieved reverse gust tunnel: ") + reverseGuestTunnel->ip + ":" + std::to_string(reverseGuestTunnel->port));

	auto reverseHostTunnel = hostPeer->openP2PTunnel("pongServer").get();
	logger->log(std::string("Retrieved reverse host tunnel: ") + reverseHostTunnel->ip + ":" + std::to_string(reverseHostTunnel->port));
	UdpSocket reverseHostSocket(logger, 0, false);
	reverseHostSocket.Send(reverseHostTunnel->port, "blih");

	hostScene->disconnect().wait();
	guestScene->disconnect().wait();



	logger->log("Disconnected host and guest scenes");
}

//void p2pClient(std::string endpoint, std::string account, std::string application, std::string sceneId)
//{
//	auto logger = std::make_shared<Stormancer::ConsoleLogger>();
//	//auto rakPeer = RakNet::RakPeerInterface::GetInstance();
//	//UdpSocket testSocket(logger, 0);
//	auto config = Stormancer::Configuration::create(endpoint, account, application);
//	config->logger = logger;
//
//	auto client = Stormancer::IClient::create(config);
//	auto scene = client->connectToPublicScene(sceneId)
//		.then([logger](pplx::task<std::shared_ptr<Stormancer::Scene>> sceneTask)
//	{
//		try
//		{
//			return sceneTask.get();
//		}
//		catch (const std::exception& ex)
//		{
//			logger->log(ex);
//			throw;
//		}
//	}).get();
//
//	auto rpc = scene->dependencyResolver().resolve<Stormancer::RpcService>();
//	std::string p2pToken = rpc->rpc<std::string>("getP2PToken", true).get();
//	logger->log(std::string("Obtained P2P token: ") + p2pToken);
//	if (p2pToken.empty())
//	{
//		auto hostTunnel = scene->registerP2PServer("pingServer");
//		logger->log(std::string("Registered p2p server: ") + hostTunnel->ip + ":" + std::to_string(hostTunnel->port));
//
//		UdpSocket hostSocket(logger, hostTunnel->port, true);
//
//#ifndef __NX
//		std::cin.get();
//#endif
//	}
//	else
//	{
//		auto peer = scene->openP2PConnection(p2pToken).get();
//		logger->log("Opened P2P connection");
//
//		std::shared_ptr<Stormancer::P2PTunnel> tunnel = peer->openP2PTunnel("pingServer").get();
//
//		logger->log(std::string("Opened tunnel: ") + tunnel->ip + ":" + std::to_string(tunnel->port));
//
//		UdpSocket guestSocket(logger, 0, false);
//		guestSocket.Send(tunnel->port);
//
//#ifndef __NX
//		std::cin.get();
//#endif
//	}
//}
