#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;

auto ilogger = ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
auto logger = (ConsoleLogger*)ilogger;
Scene* scene = nullptr;

Concurrency::task<void> test(Client& client)
{
	return client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		scene = t.get();

		scene->addRoute(L"echo.out", [](shared_ptr<Packet<IScenePeer>> p) {
			wstring message;
			*p->stream >> message;
			logger->logWhite(message);
		});

		return scene->connect().then([](pplx::task<void> t2) {
			if (t2.is_done())
			{
				logger->logGrey(L"Connected");
			}
			else
			{
				logger->logRed(L"Failed to connect");
			}
		});
	});
}

	int main(int argc, char* argv[])
	{
		srand(time(NULL));

		logger->logGrey(L"Connecting...");

		ClientConfiguration config(L"test", L"echo");
		config.serverEndpoint = L"http://localhost:8081";
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
		while (1)
		{
			wcin.clear();
			message.clear();
			getline(wcin, message);
			if (scene && scene->connected())
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
