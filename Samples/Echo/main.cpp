#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
shared_ptr<Scene> scene = nullptr;

pplx::task<void> test(Client& client)
{
	logger->logWhite("Get scene");
	return client.getPublicScene("main").then([](shared_ptr<Scene> sc) {
		scene = sc;
		logger->logGreen("Done");
		int nbMsgToSend = 10;
		auto nbMsgReceived = new int(0);

		logger->logWhite("Add route");
		scene->addRoute("echo", [nbMsgToSend, nbMsgReceived](shared_ptr<Packet<IScenePeer>> p) {
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
		return scene->connect().then([nbMsgToSend]() {
			logger->logGreen("Done");
			for (int i = 0; i < nbMsgToSend; i++)
			{
				scene->sendPacket("echo", [](bytestream* stream) {
					int32 number1(rand()), number2(rand()), number3(rand());
					*stream << number1 << number2 << number3;
					logger->logWhite("Sending message: [ " + to_string(number1) + " ; " + to_string(number2) + " ; " + to_string(number3) + " ]");
				});
			}
		});
	});
}

int main(int argc, char* argv[])
{
	srand((uint32)time(NULL));

	//logger->logWhite("Type 'Enter' to start the sample");
	//cin.ignore();

	logger->logWhite("Create client");
	Configuration config("997bc6ac-9021-2ad6-139b-da63edee8c58", "tester");
	//config.serverEndpoint = "http://localhost:8081";
	Client client(&config);
	logger->logGreen("Done");

	auto task = test(client);
	task.wait();

	//DefaultScheduler scheduler;
	//int i = 0;
	//auto truc = scheduler.schedulePeriodic(1000, Action<>(std::function<void()>([&i]() {
	//	std::cout << i++ << std::endl;
	//})));

	cin.ignore();

	return 0;
}
