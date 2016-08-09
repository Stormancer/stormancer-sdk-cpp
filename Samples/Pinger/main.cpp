#include <stormancer.h>
#include "ConsoleLogger.h"
#include "NatPlugin.h"
#include "headers.h"
auto logger = (ConsoleLogger*)Stormancer::ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));

class MyClient
{
public:
	Stormancer::Configuration* config = nullptr;
	Stormancer::Client* client = nullptr;
	Stormancer::Scene* scene = nullptr;

	MyClient* host = nullptr;
	bool isHost = false;
	std::string userId;
	std::string name;
	std::function<void()> clean;
};

MyClient* myHost = nullptr;
MyClient* myClient = nullptr;

MyClient* create_host();
MyClient* create_client();
void test_nat();

MyClient* create_client()
{
	auto myClient = new MyClient();

	static int sid = 0;
	myClient->name = "client " + std::to_string(sid++);

	auto config = Stormancer::Configuration::forAccount("ee59dae9-332d-519d-070e-f9353ae7bbce", "battlefeet-gothic");
	//config->addPlugin(new MessagingPlugin());
	config->addPlugin(new NatPlugin());
	config->synchronisedClock = false;
	config->maxPeers = 5;
	auto client = Stormancer::Client::createClient(config);

	myClient->config = config;
	myClient->client = client;

	auto authService = client->dependencyResolver()->resolve<Stormancer::IAuthenticationService>();
	auto steamTicket = "SteamTicket " + myClient->name;
	authService->steamLogin(steamTicket.c_str()).then([client, authService, myClient](Stormancer::Result<Stormancer::Scene*>* result) {
		if (result->success())
		{
			myClient->userId = authService->userId();

			auto sceneProfiles = result->get();
			logger->log(Stormancer::LogLevel::Info, "create_client", "Steam authentication OK", myClient->name.c_str());

			logger->log(Stormancer::LogLevel::Info, "create_client", "Profiles scene connect", "");
			sceneProfiles->connect().then([client, authService, sceneProfiles, myClient](Stormancer::Result<>* result) {
				if (result->success())
				{
					logger->log(Stormancer::LogLevel::Info, "create_client", "Profiles scene connect OK", myClient->name.c_str());

					auto natService = client->dependencyResolver()->resolve<NatService>();

					natService->onNewP2PConnection([myClient, natService](RakNetPeer* rakNetPeer) {
						auto msg = std::string() + "New P2P connection. RakNetGUID = " + rakNetPeer->rakNetGUID().ToString();
						logger->log(Stormancer::LogLevel::Info, "create_client", msg.c_str(), myClient->name.c_str());

						RakNet::SystemAddress remoteSystems[5];
						Stormancer::uint16 numberOfSystems;
						natService->_rakPeerInterface->GetConnectionList(remoteSystems, &numberOfSystems);
						std::string systems = "Connected systems : ";
						for (auto i = 0; i < numberOfSystems; ++i)
						{
							systems += remoteSystems[i].ToString();
							systems += "; ";
						}
						logger->log(Stormancer::LogLevel::Info, "create_client", systems.c_str(), myClient->name.c_str());

						rakNetPeer->onConnectionStateChanged([myClient](Stormancer::ConnectionState state) {
							auto stateStr = std::to_string((int)state);
							logger->log(Stormancer::LogLevel::Info, "create_client", "RakNetPeer onConnectionStateChanged", stateStr.c_str());
						});

						rakNetPeer->onPacketReceived([myClient](void* data) {
							RakNet::Packet& packet = *(RakNet::Packet*)data;
							std::string message((char*)packet.data, packet.length);
							message = "NAT packet received " + message;
							logger->log(Stormancer::LogLevel::Info, "create_client", message.c_str(), myClient->name.c_str());
						});

						if (!myClient->isHost)
						{
							rakNetPeer->sendPacket([myClient](Stormancer::bytestream* stream) {
								*stream << "salut";
							});
						}
					});

					auto guid = std::string() + "myRakNetGUID = " + natService->myRakNetGUID.ToString();
					logger->log(Stormancer::LogLevel::Info, "create_client", guid.c_str(), myClient->name.c_str());

					if (myClient->isHost)
					{
						//
					}
					else
					{
						while (1)
						{
							Sleep(1000);
							if (myHost && myClient)
							{
								if (myHost->userId.size())
								{
									break;
								}
							}
						}
						logger->log(Stormancer::LogLevel::Info, "create_client", "WAIT FINISHED, START NAT PUNCH THROUGH", myClient->name.c_str());
						auto hostId = myHost->userId;
						
						natService->openNat(hostId.c_str()).then([myClient](Stormancer::Result<RakNetPeer*>* result) {
							if (result->success())
							{
								logger->log(Stormancer::LogLevel::Info, "create_client", "openNat succeed", myClient->name.c_str());
								auto rakNetPeer = result->get();
								logger->log(Stormancer::LogLevel::Info, "create_client", "NAT send packet", myClient->name.c_str());
							}
							else
							{
								logger->log(Stormancer::LogLevel::Error, "create_client", "openNat failed", myClient->name.c_str());
							}
							delete result;
						});
					}
				}
				else
				{
					auto msg = std::string() + "Profiles scene connect failed" + result->reason();
					logger->log(Stormancer::LogLevel::Info, "create_client", msg.c_str(), myClient->name.c_str());
				}
				Stormancer::destroy(result);
			});

			myClient->clean = [authService, client]() {
				authService->logout().then([client](Stormancer::Result<>* result) {
					client->disconnect();
					Stormancer::destroy(result);
				});
			};
		}
		else
		{
			logger->log(Stormancer::LogLevel::Info, "create_client", "Steam authentication failed", result->reason());
		}
		Stormancer::destroy(result);
	});

	return myClient;
}

void test_nat()
{
	myHost = create_client();
	myHost->name = "host";
	myHost->isHost = true;
	myClient = create_client();
	myClient->name = "client 1";
}

int main()
{
	test_nat();

	std::cin.ignore();

	return 0;
}
