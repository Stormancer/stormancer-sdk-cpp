
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
	void Send(unsigned short port)
	{
		RakNet::RNS2_SendParameters sp;
		sp.data = (char*)"blah";
		sp.length = 5;
		sp.systemAddress.FromStringExplicitPort("127.0.0.1", port);
		socket->Send(&sp, _FILE_AND_LINE_);
	}

	// Hérité via RNS2EventHandler
	virtual void OnRNS2Recv(RakNet::RNS2RecvStruct * recvStruct) override
	{
		if (recvStruct->systemAddress.GetPort() != socket->GetBoundAddress().GetPort())
		{
			_logger->log("P2P sample: received a message: " + std::string(recvStruct->data));
			if (_server)
			{
				Send(recvStruct->systemAddress.GetPort());
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

void p2pConnect(std::shared_ptr<Stormancer::IClient> hostClient, std::shared_ptr<Stormancer::IClient> guestClient, Stormancer::ILogger_ptr logger, std::string sceneId)
{
	std::shared_ptr<Stormancer::Scene> hostScene;
	std::shared_ptr<Stormancer::Scene> guestScene;

	try
	{
		hostScene = hostClient->connectToPublicScene(sceneId).get();
	}
	catch (const std::exception& ex)
	{
		logger->log(ex);
		throw;
	}

	try
	{
		guestScene = guestClient->connectToPublicScene(sceneId).get();
	}
	catch (const std::exception& ex)
	{
		logger->log(ex);
		throw;
	}

	auto hostRpc = hostScene->dependencyResolver().resolve<Stormancer::RpcService>();
	auto guestRpc = guestScene->dependencyResolver().resolve<Stormancer::RpcService>();
	std::string p2pToken = guestRpc->rpc<std::string>("getP2PToken", true).get();

	logger->log(std::string("Got P2P token ") + p2pToken);

	auto hostTunnel = hostScene->registerP2PServer("pingServer");
	UdpSocket hostSocket(logger, hostTunnel->port, true);

	logger->log(std::string("Retrieved host tunnel: ") + hostTunnel->ip + ":" + std::to_string(hostTunnel->port));

	auto guestPeer = guestScene->openP2PConnection(p2pToken).get();
	logger->log("Retrieved guestPeer.");

	auto guestTunnel = guestPeer->openP2PTunnel("pingServer").get();
	
	logger->log(std::string("Retrieved guest tunnel: ") + guestTunnel->ip + ":" + std::to_string(guestTunnel->port));
	UdpSocket guestSocket(logger, 0, false);
	guestSocket.Send(guestTunnel->port);

	

	hostScene->disconnect().wait();
	guestScene->disconnect().wait();

	logger->log("Disconnected host and guest scenes");
}

void p2pClient(std::string endpoint, std::string account, std::string application, std::string sceneId)
{
	auto logger = std::make_shared<Stormancer::ConsoleLogger>();
	//auto rakPeer = RakNet::RakPeerInterface::GetInstance();
	//UdpSocket testSocket(logger, 0);
	auto config = Stormancer::Configuration::create(endpoint, account, application);
	config->logger = logger;

	auto client = Stormancer::IClient::create(config);
	auto scene = client->connectToPublicScene(sceneId)
		.then([logger](pplx::task<std::shared_ptr<Stormancer::Scene>> sceneTask)
	{
		try
		{
			return sceneTask.get();
		}
		catch (const std::exception& ex)
		{
			logger->log(ex);
			throw;
		}
	}).get();

	auto rpc = scene->dependencyResolver().resolve<Stormancer::RpcService>();
	std::string p2pToken = rpc->rpc<std::string>("getP2PToken", true).get();
	logger->log(std::string("Obtained P2P token: ") + p2pToken);
	if (p2pToken.empty())
	{
		auto hostTunnel = scene->registerP2PServer("pingServer");
		logger->log(std::string("Registered p2p server: ") + hostTunnel->ip + ":" + std::to_string(hostTunnel->port));

		UdpSocket hostSocket(logger, hostTunnel->port, true);


		std::cin.get();

	}
	else
	{
		auto peer = scene->openP2PConnection(p2pToken).get();
		logger->log("Opened P2P connection");

		std::shared_ptr<Stormancer::P2PTunnel> tunnel = peer->openP2PTunnel("pingServer").get();

		logger->log(std::string("Opened tunnel: ") + tunnel->ip + ":" + std::to_string(tunnel->port));

		UdpSocket guestSocket(logger, 0, false);
		guestSocket.Send(tunnel->port);


		std::cin.get();

	}
}
