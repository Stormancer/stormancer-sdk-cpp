#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
shared_ptr<Scene> scene = nullptr;

pplx::task<void> test(Client& client)
{
	logger->logWhite("Get scene");
	return client.getPublicScene("main").then([&client](Scene_ptr sc) {
		scene = sc;
		logger->logGreen("Done");

		logger->logWhite("Add route");
		scene->addRoute("echo", [](Packetisp_ptr p) {
			long clock;
			*p->stream >> clock;
			logger->logGreen("Received message: " + to_string(clock));
		});
		logger->logGreen("Done");

		logger->logWhite("Connect to scene");
		return scene->connect().then([&client]() {
			logger->logGreen("Done");

			logger->logWhite("Displaying sync clock results");
		});
	});
}

int main(int argc, char* argv[])
{
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
		int test = 0;
	}

	cin.ignore();

	return 0;
}
