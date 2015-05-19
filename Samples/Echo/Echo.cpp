#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;

auto ilogger = ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
auto logger = (ConsoleLogger*)ilogger;

Concurrency::task<void> test(Client& client)
{
	logger->logWhite(L"Get scene");
	return client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		Scene* scene = t.get();
		logger->logGreen(L"Done");
		int nbMsgToSend = 10;
		auto nbMsgReceived = new int(0);

		logger->logWhite(L"Add route");
		scene->addRoute(L"echo.out", [scene, nbMsgToSend, nbMsgReceived](shared_ptr<Packet<IScenePeer>> p) {
			int32 number1, number2, number3;
			*p->stream >> number1 >> number2 >> number3;
			logger->logGreen(L"Received message: [ " + to_wstring(number1) + L" ; " + to_wstring(number2) + L" ; " + to_wstring(number3) + L" ]");
			
			(*nbMsgReceived)++;
			if (*nbMsgReceived == nbMsgToSend)
			{
				logger->logGreen(L"Done");

				logger->logWhite(L"Disconnect");
				scene->disconnect().then([scene, nbMsgReceived](pplx::task<void> t3) {
					if (t3.is_done())
					{
						logger->logGreen(L"Done");
					}
					else
					{
						logger->logRed(L"Fail");
					}
					delete nbMsgReceived;
					logger->logWhite(L"Type 'Enter' to finish the sample");
				});
			}
		});
		logger->logGreen(L"Done");

		logger->logWhite(L"Connect to scene");
		return scene->connect().then([scene, nbMsgToSend](pplx::task<void> t2) {
			if (t2.is_done())
			{
				logger->logGreen(L"Done");
				for (int i = 0; i < nbMsgToSend; i++)
				{
					scene->sendPacket(L"echo.in", [](bytestream* stream) {
						int32 number1(rand()), number2(rand()), number3(rand());
						*stream << number1 << number2 << number3;
						logger->logWhite(L"Sending message: [ " + to_wstring(number1) + L" ; " + to_wstring(number2) + L" ; " + to_wstring(number3) + L" ]");
					});
				}
			}
			else
			{
				logger->logRed(L"Fail");
			}
		});
	});
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	//logger->logWhite(L"Type 'Enter' to start the sample");
	//cin.ignore();

	logger->logWhite(L"Create client");
	ClientConfiguration config(L"test", L"echo");
	config.serverEndpoint = L"http://localhost:8081";
	Client client(&config);
	logger->logGreen(L"Done");

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
