#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Error));
shared_ptr<Scene> scene = nullptr;
bool exitProgram = false;

pplx::task<void> test(Client& client)
{
	return client.getPublicScene("main").then([](shared_ptr<Scene> sc) {
		scene = sc;

		scene->addRoute("echo", [](shared_ptr<Packet<IScenePeer>> p) {
			string message;
			*p->stream >> message;
			logger->logWhite(message);
		});

		return scene->connect().then([]() {
			logger->logGrey("Connected");
		});
	});
}

int main(int argc, char* argv[])
{
	srand((uint32)time(NULL));

	logger->logGrey("Connecting...");
	logger->logGrey("You can exit the program by typing 'exit'");

	Configuration config("997bc6ac-9021-2ad6-139b-da63edee8c58", "tester");
	Client client(&config);

	auto task = test(client);

	try
	{
		task.wait();
	}
	catch (const exception &e)
	{
		logger->log(e);
	}

	string message;
	while (!exitProgram)
	{
		cin.clear();
		message.clear();
		getline(cin, message);

		if (message == "exit")
		{
			exitProgram = true;
		}
		else if (scene && scene->connected())
		{
			scene->sendPacket("echo", [message](bytestream* stream) {
				*stream << message;
			});
		}
	}

	if (scene)
	{
		scene->disconnect();
	}

	return 0;
}
