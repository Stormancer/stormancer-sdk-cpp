#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
Scene_ptr scene;
bool stop = false;
const std::string rpcData = "stormancer";
const std::string accountId = "997bc6ac-9021-2ad6-139b-da63edee8c58";
//const std::string accountId = "test";
const std::string applicationName = "tester";
const std::string sceneName = "main";

pplx::task<void> test(Client& client)
{
	logger->logWhite("Get scene");
	return client.getPublicScene(sceneName).then([&client](Scene_ptr sc) {
		scene = sc;
		logger->logGreen("Done");
		int nbMsgToSend = 10;
		auto nbMsgReceived = new int(0);

		logger->logWhite("Add route");
		scene->addRoute("echo", [nbMsgToSend, nbMsgReceived](Packetisp_ptr p) {
			int32 number1, number2, number3;
			*p->stream >> number1 >> number2 >> number3;
			logger->logGreen("Received message: [ " + to_string(number1) + " ; " + to_string(number2) + " ; " + to_string(number3) + " ]");

			(*nbMsgReceived)++;
			if (*nbMsgReceived == nbMsgToSend)
			{
				logger->logGreen("Done");

				logger->logWhite("Disconnect");
				scene->disconnect().then([nbMsgReceived]() {
					logger->logGreen("Done");
					delete nbMsgReceived;
					logger->logWhite("Type 'Enter' to finish the sample");
				});
			}
		});
		logger->logGreen("Done");

		auto rpcService = (RpcService*)scene->getComponent("rpcService");
		rpcService->addProcedure("rpc", [](RpcRequestContex_ptr rc) {
			logger->logWhite("rpc request received");
			pplx::task_completion_event<void> tce;
			std::string message;
			*rc->inputStream() >> message;
			if (message == "stormancer")
			{
				logger->logGreen("rpc server OK");
				logger->logWhite("sending rpc response");
				rc->sendValue(Action<bytestream*>([message](bytestream* bs) {
					*bs << message;
				}), PacketPriority::MEDIUM_PRIORITY);
				tce.set();
			}
			else
			{
				logger->logRed("rpc server failed");
				tce.set_exception<std::runtime_error>(std::runtime_error("bad rpc server request"));
			}
			return pplx::create_task(tce);
		}, true);

		logger->logWhite("Connect to scene");
		return scene->connect().then([&client, nbMsgToSend, rpcService](pplx::task<void> t) {
			logger->logGreen("Done");

			pplx::create_task([&client, rpcService]() {
				//while (!stop)
				//{
				/*
				int64 clock = client.clock();
				logger->logYellow("clock: " + to_string(clock) + " ms (" + to_string(clock / 1000.0) + " s)");

				try
				{
				scene->sendPacket("echo", [](bytestream* stream) {
				int32 number1(rand()), number2(rand()), number3(rand());
				*stream << number1 << number2 << number3;
				logger->logWhite("Sending message: [ " + to_string(number1) + " ; " + to_string(number2) + " ; " + to_string(number3) + " ]");
				});
				}
				catch (const std::exception& e)
				{
				// connect failed
				int test = 0;
				}
				*/

				logger->logWhite("sending rpc request");
				auto subscriber = rpcService->rpc("rpc", Action<bytestream*>([](bytestream* bs) {
					msgpack::pack(bs, rpcData);
				}), PacketPriority::MEDIUM_PRIORITY).subscribe([](Packetisp_ptr packet) {
					logger->logWhite("rpc response received");
					uint64 serverClock;
					*packet->stream >> serverClock;
				});

				//Sleep(1000);
				//}
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
	logger->logGreen("Done");

	auto task = test(client);
	try
	{
		task.wait();
	}
	catch (const std::exception& e)
	{
		// create client failed
	}

	cin.ignore();
	stop = true;

	return 0;
}
