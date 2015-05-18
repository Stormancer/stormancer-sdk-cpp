#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;

auto ilogger = ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Info));
auto logger = (ConsoleLogger*)ilogger;
Scene* scene = nullptr;

//auto logger = new ConsoleLogger(Stormancer::LogLevel::Trace);

Concurrency::task<void> test1(Client& client)
{
	logger->logWhite(L"Get scene");
	return client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		scene = t.get();
		logger->logGreen(L"Done");

		logger->logWhite(L"Add 'echo.out' route");
		scene->addRoute(L"echo.out", [](shared_ptr<Packet<IScenePeer>> p) {
			wstring message;
			*p->stream >> message;

			if (message == L"hello world!")
			{
				logger->logGreen(L"Done");

				logger->logWhite(L"Disconnect");

				//client->disconnect(scene);
			    //delete scene;
				scene->disconnect().then([](pplx::task<void> t3) {
					if (t3.is_done())
					{
						logger->logGreen(L"Done");
					}
					else
					{
						logger->logRed(L"Failed");
					}
				});
			}
			else
			{
				logger->logRed(L"Bad data received");
			}
		});
		logger->logGreen(L"Done");

		logger->logWhite(L"Connect");
		return scene->connect().then([](pplx::task<void> t2) {
			if (t2.is_done())
			{
				logger->logGreen(L"Done");

				logger->logWhite(L"Send Message");
				scene->sendPacket(L"echo.in", [](bytestream* stream) {
					*stream << L"hello world!";
				});
				logger->logGreen(L"Done");

				logger->logWhite(L"Receive Message");
			}
			else
			{
				logger->logRed(L"Bad stuff happened...");
			}
		});
	});
}

Concurrency::task<void> test2(Client& client)
{
	return client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		scene = t.get();
		scene->addRoute(L"echo.out", [](shared_ptr<Packet<IScenePeer>> p) {
			int32 number;
			*p->stream >> number;
			//logger->logGreen(L"Received message: " + to_wstring(number));
		});

		logger->log(L"Connecting to scene!");
		return scene->connect().then([](pplx::task<void> t2) {
			if (t2.is_done())
			{
				logger->log(L"Connected to scene!");
				while(1)
				//for (int i = 0; i < 100; i++)
				{
					scene->sendPacket(L"echo.in", [](bytestream* stream) {
						int32 number = rand();
						*stream << number;
						//logger->logWhite(L"Sending message: " + to_wstring(number));
					});
				}
			}
			else
			{
				logger->logRed(L"Bad stuff happened...");
			}
		});
	});
}

int main(int argc, char* argv[])
{
	cin.ignore();

	srand(time(NULL));

	std::set_terminate([]() {
		bool termin = true;
	});

	logger->logWhite(L"Creating client");
	ClientConfiguration config(L"test", L"echo");
	config.serverEndpoint = L"http://localhost:8081";
	Client client(&config);
	logger->logGreen(L"Done");

	//auto task = test1(client);
	auto task = test2(client);

	try
	{
		task.wait();
	}
	catch (const exception &e)
	{
		logger->log(e);
	}

	cin.ignore();

	if (scene)
	{
		delete scene;
	}

	return 0;
}
