#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Error));
shared_ptr<Scene> scene = nullptr;
bool exitProgram = false;

pplx::task<void> test(Client& client)
{
	return client.getPublicScene(L"test-scene").then([](shared_ptr<Scene> sc) {
		scene = sc;

		scene->addRoute(L"echo.out", [](shared_ptr<Packet<IScenePeer>> p) {
			wstring message;
			*p->stream >> message;
			logger->logWhite(message);
		});

		return scene->connect().then([]() {
			logger->logGrey(L"Connected");
		});
	});
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	logger->logGrey(L"Connecting...");
	logger->logGrey(L"You can exit the program by typing 'exit'");

	Configuration config(L"997bc6ac-9021-2ad6-139b-da63edee8c58", L"echo");
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

	wstring message;
	while (!exitProgram)
	{
		wcin.clear();
		message.clear();
		getline(wcin, message);

		if (message == L"exit")
		{
			exitProgram = true;
		}
		else if (scene && scene->connected())
		{
			scene->sendPacket(L"echo.in", [message](bytestream* stream) {
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
