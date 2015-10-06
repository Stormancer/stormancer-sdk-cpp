#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
Scene_ptr scene;
bool stop = false;

pplx::task<void> test(Client& client)
{
	logger->logWhite("Get scene");
	return client.getPublicScene("main").then([&client](Scene_ptr sc) {
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

		logger->logWhite("Connect to scene");
		return scene->connect().then([&client, nbMsgToSend](pplx::task<void> t) {
			logger->logGreen("Done");

			pplx::create_task([&client]() {
				while (!stop)
				{
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

					Sleep(5000);
				}
			});

		});
	});
}

int main(int argc, char* argv[])
{
	srand((uint32)time(NULL));

	logger->logWhite("Create client");
	Configuration config("997bc6ac-9021-2ad6-139b-da63edee8c58", "tester");
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
		int test = 0;
	}
	logger->logGreen("CONNECTED");

	cin.ignore();
	stop = true;

	return 0;
}
