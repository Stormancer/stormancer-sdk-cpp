/*
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

void rpc(Scene* scene)
{
	auto rpcService = scene->dependencyResolver()->resolve<IRpcService>();

	ISubscription* sub = nullptr;

	logger->log(Stormancer::LogLevel::Info, "rpc", "start rpc...", "");
	auto observable = rpcService->rpc("rpc", [](bytestream* stream) {}, PacketPriority::MEDIUM_PRIORITY);

	auto onNext = [&sub, scene](Packetisp_ptr packet) {
		logger->log(Stormancer::LogLevel::Info, "rpc", "observable next", "");
		//sub->destroy();
		//rpc(scene);
	};

	auto onComplete = []() {
		logger->log(Stormancer::LogLevel::Trace, "rpc", "observable complete", "");
	};

	auto onError = [](const char* error) {
		logger->log(Stormancer::LogLevel::Error, "rpc", "observable error", error);
	};

	sub = observable->subscribe(onNext, onError, onComplete);
}

std::string connectionStateToString(ConnectionState connectionState)
{
	std::string stateStr = std::to_string((int)connectionState) + " ";
	switch (connectionState)
	{
	case ConnectionState::Disconnected:
		stateStr += "Disconnected";
		break;
	case ConnectionState::Connecting:
		stateStr += "Connecting";
		break;
	case ConnectionState::Connected:
		stateStr += "Connected";
		break;
	case ConnectionState::Disconnecting:
		stateStr += "Disconnecting";
		break;
	}
	return stateStr;
}

int main()
{
	{
		logger->log(LogLevel::Debug, "main", "Create client", "");
		config = Configuration::forAccount(accountId, applicationName);
		config->synchronisedClock = false;
		client = Client::createClient(config);
		logger->log(LogLevel::Info, "main", "Create client OK", "");

		client->onConnectionStateChanged([](ConnectionState connectionState) {
			auto stateStr = connectionStateToString(connectionState);
			logger->log(LogLevel::Info, "main", "CLIENT ", stateStr.c_str());
		});

		client->getPublicScene(sceneName).then([](Result<Scene*>* result) {
			if (result->success())
			{
				sceneMain = result->get();

				sceneMain->onConnectionStateChanged([](ConnectionState connectionState) {
					auto stateStr = connectionStateToString(connectionState);
					logger->log(LogLevel::Info, "main", "SCENE MAIN ", stateStr.c_str());
				});

				sceneMain->connect().then([](Result<>* result2) {
					if (result2->success())
					{
						logger->log(LogLevel::Info, "main", "Connect OK", "");

						//rpc(sceneMain);

						//logger->log(LogLevel::Info, "main", "Scene disconnect", "");
						//sceneMain->disconnect().then([](Result<>* result) {
						//	if (result->success())
						//	{
						//		logger->log(LogLevel::Info, "main", "Scene disconnect OK", result->reason());
						//	}
						//	else
						//	{
						//		logger->log(LogLevel::Warn, "main", "Scene disconnect failed", result->reason());
						//	}
						//	destroy(result);
						//});

						client->disconnect();
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
*/