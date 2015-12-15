#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
const char* accountId = "ee59dae9-332d-519d-070e-f9353ae7bbce";
const char* applicationName = "battlefeet-gothic";
Configuration* config = nullptr;
Client* client = nullptr;
Scene* sceneMain = nullptr;
IRpcService* rpcService = nullptr;

void rpc()
{
	static unsigned int i = 1;
	auto istr = std::to_string(++i);

	logger->log(Stormancer::LogLevel::Info, "rpc", "start rpc...", istr.c_str());
	auto obs = rpcService->rpc("profiles.getall", [](bytestream* stream) {}, PacketPriority::MEDIUM_PRIORITY);
	auto onNext = [istr](Packetisp_ptr packet) {
		logger->log(Stormancer::LogLevel::Info, "rpc", "observable next", istr.c_str());
		rpc();
	};
	auto onComplete = [istr]() {
		logger->log(Stormancer::LogLevel::Trace, "rpc", "observable complete", istr.c_str());
	};
	auto onError = [istr](const char* error) {
		logger->log(Stormancer::LogLevel::Error, "rpc", "observable error", error);
	};
	auto sub = obs->subscribe(onNext, onError, onComplete);
}

int main()
{
	//{
	//	Stormancer::MsgPackMaybe<int> mb1;
	//	mb1 = new int(127);
	//	std::stringstream ss;
	//	msgpack::pack(ss, mb1);
	//	std::string str = ss.str();
	//	msgpack::unpacked ret;
	//	msgpack::unpack(ret, str.data(), str.size());
	//	Stormancer::MsgPackMaybe<int> mb2;
	//	ret.get().convert(&mb2);
	//	auto b1 = mb2.hasValue();
	//	if (b1)
	//	{
	//		auto i1 = mb2.get();
	//		std::cout << *i1;
	//	}
	//	else
	//	{
	//		std::cout << "nil";
	//	}
	//}

	{
		logger->log(LogLevel::Debug, "test_connect", "Create client", "");
		config = Configuration::forAccount(accountId, applicationName);
		config->synchronisedClock = false;
		client = Client::createClient(config);
		logger->log(LogLevel::Info, "test_connect", "Create client OK", "");

		auto authService = client->dependencyResolver()->resolve<IAuthenticationService>();
		authService->steamLogin("SteamTicket").then([authService](Result<Scene*>* result) {
			logger->log(LogLevel::Info, "test_steam", "Steam authentication OK", "");
			if (result->success())
			{
				sceneMain = result->get();
				sceneMain->connect().then([](Result<>* result) {
					if (result->success())
					{
						logger->log(LogLevel::Info, "test_connect", "Connect OK", "");

						rpcService = sceneMain->dependencyResolver()->resolve<IRpcService>();
						rpc();
					}
					else
					{
						logger->log(LogLevel::Error, "test_connect", "Failed to connect to the scene", result->reason());
					}
					result->destroy();
				});
			}
			else
			{
				logger->log(LogLevel::Error, "test_steam", "Failed to logout", result->reason());
			}
			result->destroy();
		});
	}

	std::cin.ignore();

	return 0;
}
