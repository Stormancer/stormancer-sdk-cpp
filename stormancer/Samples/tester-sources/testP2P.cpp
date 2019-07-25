#include "testP2P.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Logger/ConsoleLogger.h"
#include "stormancer/IClient.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/Utilities/TaskUtilities.h"

using namespace Stormancer;

class UdpSocket : RakNet::RNS2EventHandler
{
public:

	UdpSocket(std::shared_ptr<ILogger> logger, unsigned short port, bool server) :
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
			throw std::runtime_error("P2P test : Failed to bind socket");
		}
		else if (br == RakNet::BR_FAILED_SEND_TEST)
		{
			RakNet::RakNetSocket2Allocator::DeallocRNS2(socket);
			throw std::runtime_error("P2P test : Failed to send test message");
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

	void Send(unsigned short port, const std::string& data)
	{
		RakNet::RNS2_SendParameters sp;
		sp.data = (char*)data.c_str();
		sp.length = (int)data.size() + 1;
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

	std::shared_ptr<ILogger> _logger;

	RakNet::RakNetSocket2* socket;

	bool _server;
};

struct TestContextBase
{
	virtual ~TestContextBase() = default;

	virtual void reset()
	{
		peerDisconnectedSub.reset();
		peerConnectedSub.reset();
		udpSocket.reset();
		tunnel.reset();
		scene.reset();
		client.reset();
		name.resize(0);
	}

	std::string name;
	std::shared_ptr<IClient> client;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<P2PTunnel> tunnel;
	std::shared_ptr<UdpSocket> udpSocket;
	Subscription peerConnectedSub;
	Subscription peerDisconnectedSub;
};

struct HostTestContext : public TestContextBase
{
	void reset() override
	{
		TestContextBase::reset();
	}
};

struct GuestTestContext : public TestContextBase
{
	void reset() override
	{
		reverseGuestUdpSocket.reset();
		reverseHostUdpSocket.reset();
		reverseGuestTunnel.reset();
		reverseHostTunnel.reset();
		guestPeer.reset();
		hostPeer.reset();
		hostPeerTce = pplx::task_completion_event<std::shared_ptr<IP2PScenePeer>>();
		guestReceivedMessageTce = pplx::task_completion_event<void>();
		hostReceivedMessageTce = pplx::task_completion_event<void>();

		TestContextBase::reset();
	}

	pplx::task_completion_event<void> hostReceivedMessageTce;
	pplx::task_completion_event<void> guestReceivedMessageTce;
	pplx::task_completion_event<std::shared_ptr<IP2PScenePeer>> hostPeerTce;
	std::shared_ptr<IP2PScenePeer> hostPeer;
	std::shared_ptr<IP2PScenePeer> guestPeer;
	std::shared_ptr<P2PTunnel> reverseHostTunnel;
	std::shared_ptr<P2PTunnel> reverseGuestTunnel;
	std::shared_ptr<UdpSocket> reverseHostUdpSocket;
	std::shared_ptr<UdpSocket> reverseGuestUdpSocket;
};

struct TestContext
{
	void reset()
	{
		host->reset();
		guest->reset();
		host.reset();
		guest.reset();
		sceneId.resize(0);
	}

	std::string sceneId;
	std::shared_ptr<HostTestContext> host;
	std::shared_ptr<GuestTestContext> guest;
};

pplx::task<void> hostConnect(std::shared_ptr<HostTestContext> context, std::string sceneId, std::shared_ptr<ILogger> logger);
pplx::task<void> p2pConnect(std::shared_ptr<TestContext> context, std::shared_ptr<ILogger> logger);

std::unordered_map<std::string, std::shared_ptr<TestContext>> testContextes;
std::mutex testContextesMutex;

pplx::task<void> testP2P(std::string endpoint, std::string account, std::string application, std::string sceneId, int guestsCount)
{
	if (guestsCount < 1)
	{
		return pplx::task_from_exception<void>(std::runtime_error("Not enough guests"));
	}

	// Init
	auto logger = std::make_shared<ConsoleLogger>();

	// Create host
	auto hostConfiguration = Configuration::create(endpoint, account, application);
	hostConfiguration->logger = logger;
	//hostConfiguration->encryptionEnabled = true;

	auto hostClient = IClient::create(hostConfiguration);

	auto hostContext = std::make_shared<HostTestContext>();
	hostContext->name = "H";
	hostContext->client = hostClient;

	return hostConnect(hostContext, sceneId, logger)
		.then([logger, hostContext, guestsCount, endpoint, account, application, sceneId]()
	{
		std::vector<pplx::task<void>> testsTasks;

		// Create guests and start p2pConnect
		uint16 guestServerGamePort = 7778;
		for (int i = 0; i < guestsCount; i++)
		{
			auto guestConfiguration = Configuration::create(endpoint, account, application);
			guestConfiguration->logger = logger;
			guestConfiguration->serverGamePort = guestServerGamePort;
			guestServerGamePort++;
			//guestConfiguration->encryptionEnabled = true;

			auto guestContext = std::make_shared<GuestTestContext>();
			guestContext->name = "G" + std::to_string(i + 1);
			guestContext->client = IClient::create(guestConfiguration);

			auto context = std::make_shared<TestContext>();
			context->sceneId = sceneId;
			context->host = hostContext;
			context->guest = guestContext;

			auto task = p2pConnect(context, logger);

			task.then([context, logger](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& ex)
				{
					logger->log(ex);
				}
			});

			testsTasks.push_back(task);
		}

		// wait for all p2pConnect
		return pplx::when_all(testsTasks.begin(), testsTasks.end())
			.then([hostContext, logger]()
		{
			hostContext->tunnel.reset();
			hostContext->udpSocket.reset();

			return hostContext->scene->disconnect();
		})
			.then([hostContext, logger]()
		{
			logger->log("host scene disconnected");

			hostContext->peerConnectedSub.reset();
			hostContext->peerDisconnectedSub.reset();
			hostContext->scene.reset();

			return hostContext->client->disconnect();
		})
			.then([hostContext, logger]()
		{
			logger->log("host client disconnected");

			hostContext->client.reset();
			hostContext->reset();

			logger->log(LogLevel::Info, "P2P_test", "P2P TESTS SUCCEED");
		});
	});
}

pplx::task<void> hostConnect(std::shared_ptr<HostTestContext> hostContext, std::string sceneId, std::shared_ptr<ILogger> logger)
{
	return hostContext->client->connectToPublicScene(sceneId, [logger](std::shared_ptr<Scene> scene)
	{
		scene->addRoute("test", [logger](Packetisp_ptr packet)
		{
			logger->log(LogLevel::Info, "P2P_test", "host received a scene message from a peer", packet->readObject<std::string>());
			{
				std::lock_guard<std::mutex> lg(testContextesMutex);
				auto p2pSessionId = packet->connection->connection()->key();
				auto it = testContextes.find(p2pSessionId);
				if (it == testContextes.end())
				{
					logger->log(LogLevel::Error, "testP2P", "guest test context not found", p2pSessionId);
				}
				else
				{
					it->second->guest->hostReceivedMessageTce.set();
				}
			}
		}, MessageOriginFilter::Peer);
	})
		.then([logger, hostContext](std::shared_ptr<Scene> scene)
	{
		hostContext->scene = scene;

		hostContext->peerConnectedSub = hostContext->scene->onPeerConnected().subscribe([logger](std::shared_ptr<IP2PScenePeer> peer)
		{
			logger->log(LogLevel::Info, "P2P_test", "Peer connected on host scene", peer->sessionId());

			std::lock_guard<std::mutex> lg(testContextesMutex);
			auto p2pSessionId = peer->connection()->key();
			auto it = testContextes.find(p2pSessionId);
			if (it == testContextes.end())
			{
				logger->log(LogLevel::Error, "testP2P", "guest test context not found", p2pSessionId);
			}
			else
			{
				it->second->guest->hostPeerTce.set(peer);
			}
		});

		hostContext->peerDisconnectedSub = hostContext->scene->onPeerDisconnected().subscribe([logger](std::shared_ptr<IP2PScenePeer> peer)
		{
			logger->log(LogLevel::Info, "P2P_test", "Peer disconnected from host scene", peer->sessionId());
		});

		// Tunnel
		auto hostTunnel = hostContext->scene->registerP2PServer("pingServer");
		hostContext->tunnel = hostTunnel;
		logger->log(std::string("Retrieved host tunnel: ") + hostTunnel->ip + ":" + std::to_string(hostTunnel->port));
		hostContext->udpSocket = std::make_shared<UdpSocket>(logger, hostTunnel->port, true);
	});
}

pplx::task<void> p2pConnect(std::shared_ptr<TestContext> context, std::shared_ptr<ILogger> logger)
{
	auto receivedMessageTce = context->guest->guestReceivedMessageTce;
	return context->guest->client->connectToPublicScene(context->sceneId, [receivedMessageTce, logger](std::shared_ptr<Scene> scene)
	{
		scene->addRoute("test", [receivedMessageTce, logger](Packetisp_ptr packet)
		{
			logger->log(LogLevel::Info, "P2P_test", "guest received a scene message from a peer", packet->readObject<std::string>());
			receivedMessageTce.set();
		}, MessageOriginFilter::Peer);
	})
		.then([context, logger](std::shared_ptr<Scene> scene)
	{
		context->guest->scene = scene;

		context->guest->peerConnectedSub = context->guest->scene->onPeerConnected().subscribe([logger](std::shared_ptr<IP2PScenePeer> peer)
		{
			logger->log(LogLevel::Info, "P2P_test", "Peer connected on guest scene", peer->sessionId());
		});

		context->guest->peerDisconnectedSub = context->guest->scene->onPeerDisconnected().subscribe([logger](std::shared_ptr<IP2PScenePeer> peer)
		{
			logger->log(LogLevel::Info, "P2P_test", "Peer disconnected from guest scene", peer->sessionId());
		});

		auto guestRpc = context->guest->scene->dependencyResolver().resolve<RpcService>();

		return guestRpc->rpc<std::string>("getP2PToken", true);
	})
		.then([context, logger](std::string p2pToken)
	{
		logger->log(std::string("Got P2P token ") + p2pToken);

		return context->guest->scene->openP2PConnection(p2pToken);
	})
		.then([context, logger](std::shared_ptr<IP2PScenePeer> guestPeer)
	{
		context->guest->guestPeer = guestPeer;

		auto p2pSessionId = guestPeer->connection()->key();

		{
			std::lock_guard<std::mutex> lg(testContextesMutex);
			testContextes.insert({ p2pSessionId, context });
		}

		logger->log(LogLevel::Debug, "P2P_test", "P2P connection established", p2pSessionId);

		return pplx::create_task(context->guest->hostPeerTce);
	})
		.then([context, logger](std::shared_ptr<IP2PScenePeer> hostPeer)
	{
		context->guest->hostPeer = hostPeer;
		context->guest->hostPeerTce = pplx::task_completion_event<std::shared_ptr<IP2PScenePeer>>();

		context->guest->hostPeer->send("test", [context](obytestream& stream) {
			Serializer{}.serialize(stream, std::string("message from guest " + context->guest->name));
		});

		context->guest->guestPeer->send("test", [context](obytestream& stream) {
			Serializer{}.serialize(stream, std::string("message from host " + context->host->name));
		});

		// Test comparison operators
		assert(context->guest->hostPeer != context->guest->guestPeer);
		assert(!(context->guest->hostPeer == context->guest->guestPeer));
	})
		.then([context, logger]()
	{
		// Tunnel
		context->guest->guestPeer->openP2PTunnel("pingServer")
			.then([context, logger](std::shared_ptr<P2PTunnel> guestTunnel)
		{
			context->guest->tunnel = guestTunnel;
			logger->log(std::string("Retrieved guest tunnel: ") + guestTunnel->ip + ":" + std::to_string(guestTunnel->port));
			context->guest->udpSocket = std::make_shared<UdpSocket>(logger, (uint16)0, false);
			context->guest->udpSocket->Send(guestTunnel->port, "blah from " + context->host->name);
		})
			.then([logger](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Error, "P2P_test.tunnel", ex.what());
			}
		});

		// Reverse tunnel
		auto reverseGuestTunnel = context->guest->scene->registerP2PServer("pongServer");
		context->guest->reverseGuestTunnel = reverseGuestTunnel;
		context->guest->reverseGuestUdpSocket = std::make_shared<UdpSocket>(logger, reverseGuestTunnel->port, true);
		logger->log(std::string("Retrieved reverse gust tunnel: ") + reverseGuestTunnel->ip + ":" + std::to_string(reverseGuestTunnel->port));

		context->guest->hostPeer->openP2PTunnel("pongServer")
			.then([context, logger](std::shared_ptr<P2PTunnel> reverseHostTunnel)
		{
			context->guest->reverseHostTunnel = reverseHostTunnel;
			logger->log(std::string("Retrieved reverse host tunnel: ") + reverseHostTunnel->ip + ":" + std::to_string(reverseHostTunnel->port));
			context->guest->reverseHostUdpSocket = std::make_shared<UdpSocket>(logger, (uint16)0, false);
			context->guest->reverseHostUdpSocket->Send(reverseHostTunnel->port, "blih from " + context->guest->name);
		})
			.then([logger](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Error, "P2P_test.reverseTunnel", ex.what());
			}
		});

		return when_all(pplx::create_task(context->guest->hostReceivedMessageTce), pplx::create_task(context->guest->guestReceivedMessageTce));
	})
		.then([context, logger]()
	{
		logger->log("Received host and guest messages");

		context->guest->udpSocket.reset();
		context->guest->reverseHostUdpSocket.reset();
		context->guest->reverseGuestUdpSocket.reset();
		context->guest->tunnel.reset();
		context->guest->reverseHostTunnel.reset();
		context->guest->reverseGuestTunnel.reset();

		return context->guest->guestPeer->disconnect();
	})
		.then([context, logger]()
	{
		logger->log("guest peer disconnected");

		{
			auto p2pSessionId = context->guest->guestPeer->connection()->key();
			std::lock_guard<std::mutex> lg(testContextesMutex);
			testContextes.erase(p2pSessionId);
		}

		context->guest->hostPeer.reset();
		context->guest->guestPeer.reset();

		return context->guest->scene->disconnect();
	})
		.then([context, logger]()
	{
		logger->log("guest scene disconnected");

		context->guest->peerConnectedSub.reset();
		context->guest->peerDisconnectedSub.reset();
		context->guest->scene.reset();

		return context->guest->client->disconnect();
	})
		.then([context, logger]()
	{
		logger->log("guest client disconnected");

		context->guest->client.reset();
		context->guest->reset();
	});
}
