#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
Scene_ptr scene;
bool stop = false;
const std::string stormancer = "stormancer";
const std::string accountId = "997bc6ac-9021-2ad6-139b-da63edee8c58";
//const std::string accountId = "test";
const std::string applicationName = "tester";
const std::string sceneName = "main";
pplx::task<void> syncclockTask;

pplx::task<void> test(Client& client)
{
	logger->logWhite("Get scene");
	return client.getPublicScene(sceneName).then([&client](Scene_ptr sc) {
		scene = sc;
		logger->logGreen("Get scene OK");

		logger->logWhite("Add route");
		scene->addRoute("echo", [&client](Packetisp_ptr p) {
			std::string message;
			*p->stream >> message;
			logger->logWhite("Message received (" + message + ")");
			if (message == stormancer)
			{
				logger->logGreen("Echo OK");

				logger->logWhite("sending rpc request");
				((RpcService*)scene->getComponent("rpcService"))->rpc("rpc", Action<bytestream*>([](bytestream* bs) {
					msgpack::pack(bs, stormancer);
				}), PacketPriority::MEDIUM_PRIORITY).subscribe([](Packetisp_ptr packet) {
					std::string message;
					*packet->stream >> message;
					logger->logWhite("rpc response received (" + message + ")");
					if (message == stormancer)
					{
						logger->logGreen("RPC client OK");
					}
				});
			}
		});
		logger->logGreen("Add route OK");

		logger->logWhite("Add procedure");
		((RpcService*)scene->getComponent("rpcService"))->addProcedure("rpc", [](RpcRequestContex_ptr rc) {
			pplx::task_completion_event<void> tce;
			std::string message;
			*rc->inputStream() >> message;
			logger->logWhite("rpc request received (" + message + ")");
			if (message == stormancer)
			{
				logger->logGreen("RPC server OK");
				logger->logWhite("sending rpc response");
				rc->sendValue(Action<bytestream*>([message](bytestream* bs) {
					*bs << message;
				}), PacketPriority::MEDIUM_PRIORITY);
				tce.set();
			}
			else
			{
				logger->logRed("rpc server failed");
				tce.set_exception<std::runtime_error>(std::runtime_error("bad rpc server request (bad message)"));
			}
			return pplx::create_task(tce);
		}, true);
		logger->logGreen("Add procedure OK");

		logger->logWhite("Connect to scene");
		return scene->connect().then([&client](pplx::task<void> t) {
			logger->logGreen("Connect OK");

			try
			{
				scene->sendPacket("echo", [](bytestream* stream) {
					*stream << stormancer;
					logger->logWhite("Sending message...");
				});
			}
			catch (const std::exception& e)
			{
				logger->logRed(std::string("Can't send data to the scene through the 'echo' route.\n") + e.what());
			}

			syncclockTask = pplx::create_task([&client]() {
				while (!client.lastPing() && !stop)
				{
					Sleep(100);
				}
				if (client.lastPing())
				{
					int64 clock = client.clock();
					if (clock)
					{
						logger->logYellow("clock: " + to_string(clock / 1000.0));
						logger->logGreen("SyncClock OK");
					}
				}
			});
		});
	});
}

int main(int argc, char* argv[])
{
	srand((uint32)time(NULL));

	logger->logWhite("Create client");
	Configuration config(accountId, applicationName);
	//config.serverEndpoint = "http://localhost:8081";
	Client client(&config);
	logger->logGreen("Create client OK");

	auto task = test(client);
	try
	{
		task.wait();
	}
	catch (const std::exception& e)
	{
		logger->logRed(std::string("Client creation failed.\n") + e.what());
	}

	cin.ignore();
	stop = true;
	syncclockTask.wait();

	return 0;
}
