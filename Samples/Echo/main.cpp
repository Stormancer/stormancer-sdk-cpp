#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
shared_ptr<Scene> scene = nullptr;

pplx::task<void> test(Client& client)
{
	logger->logWhite("Get scene");
	return client.getPublicScene("main").then([&client](shared_ptr<Scene> sc) {
		scene = sc;
		logger->logGreen("Done");
		int nbMsgToSend = 10;
		auto nbMsgReceived = new int(0);

		logger->logWhite("Add route");
		scene->addRoute("echo", [nbMsgToSend, nbMsgReceived](shared_ptr<Packet<IScenePeer>> p) {
			long clock;
			*p->stream >> clock;
			logger->logGreen("Received message: " + to_string(clock));

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
		return scene->connect().then([&client, nbMsgToSend]() {
			logger->logGreen("Done");

			for (int i = 0; i < nbMsgToSend; )
			{
				int64 clock = client.clock();
				if (clock)
				{
					scene->sendPacket("echo", [&client](bytestream* stream) {
						auto clock = client.clock();
						*stream << clock;
						logger->logWhite("Sending message: " + to_string(clock));
					});
					i++;
				}
				Sleep(100);
			}
		});
	});
}

int main(int argc, char* argv[])
{
	logger->logWhite("Type 'Enter' to start the sample");
	cin.ignore();

	logger->logWhite("Create client");
	Configuration config("997bc6ac-9021-2ad6-139b-da63edee8c58", "tester");
	Client client(&config);
	logger->logGreen("Done");

	auto task = test(client);

	try
	{
		task.wait();
	}
	catch (const exception &e)
	{
		logger->log(e);
	}

	cin.ignore();

	return 0;
}
