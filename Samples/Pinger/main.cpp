#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
const char* accountId = "997bc6ac-9021-2ad6-139b-da63edee8c58";
const char* applicationName = "tester";
const char* sceneName = "main";
Configuration* config = nullptr;
Client* client = nullptr;
Scene* sceneMain = nullptr;
IRpcService* rpcService = nullptr;

void rpc()
{
	ISubscription* sub = nullptr;

	logger->log(Stormancer::LogLevel::Info, "rpc", "start rpc...", "");
	auto obs = rpcService->rpc("rpc", [](bytestream* stream) {}, PacketPriority::MEDIUM_PRIORITY);

	auto onNext = [&sub](Packetisp_ptr packet) {
		logger->log(Stormancer::LogLevel::Info, "rpc", "observable next", "");
		rpc();
		sub->destroy();
	};

	auto onComplete = []() {
		logger->log(Stormancer::LogLevel::Trace, "rpc", "observable complete", "");
	};

	auto onError = [](const char* error) {
		logger->log(Stormancer::LogLevel::Error, "rpc", "observable error", error);
	};

	sub = obs->subscribe(onNext, onError, onComplete);
}

int main()
{
	{
		logger->log(LogLevel::Debug, "main", "Create client", "");
		config = Configuration::forAccount(accountId, applicationName);
		config->synchronisedClock = false;
		client = Client::createClient(config);
		logger->log(LogLevel::Info, "main", "Create client OK", "");

		client->getPublicScene(sceneName).then([](Result<Scene*>* result) {
			if (result->success())
			{
				sceneMain = result->get();

				sceneMain->onConnectionStateChanged([](ConnectionState connectionState) {
					auto stateStr = std::to_string((int)connectionState);
					logger->log(LogLevel::Info, "main", "SCENE MAIN CONNECTION STATE CHANGED", stateStr.c_str());
				});

				sceneMain->connect().then([](Result<>* result2) {
					if (result2->success())
					{
						logger->log(LogLevel::Info, "main", "Connect OK", "");

						rpcService = sceneMain->dependencyResolver()->resolve<IRpcService>();
						rpc();
					}
					else
					{
						logger->log(LogLevel::Error, "main", "Failed to connect to the scene", result2->reason());
					}
					destroy(result2);
				});
			}
			else
			{
				logger->log(LogLevel::Error, "main", "Failed to get the scene", result->reason());
			}
			destroy(result);
		});
	}

	std::cin.ignore();

	return 0;
}
