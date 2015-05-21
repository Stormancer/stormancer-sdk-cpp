#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
shared_ptr<Scene> scene = nullptr;

pplx::task<void> test(Client& client)
{
	logger->logWhite(L"Get scene");
	return client.getPublicScene(L"test-scene").then([](shared_ptr<Scene> sc) {
		scene = sc;
		logger->logGreen(L"Done");
		int nbMsgToSend = 10;
		auto nbMsgReceived = new int(0);

		logger->logWhite(L"Add route");
		scene->addRoute(L"echo.out", [nbMsgToSend, nbMsgReceived](shared_ptr<Packet<IScenePeer>> p) {
			int32 number1, number2, number3;
			*p->stream >> number1 >> number2 >> number3;
			logger->logGreen(L"Received message: [ " + to_wstring(number1) + L" ; " + to_wstring(number2) + L" ; " + to_wstring(number3) + L" ]");

			(*nbMsgReceived)++;
			if (*nbMsgReceived == nbMsgToSend)
			{
				logger->logGreen(L"Done");

				logger->logWhite(L"Disconnect");
				scene->disconnect().then([nbMsgReceived]() {
					logger->logGreen(L"Done");
					delete nbMsgReceived;
					logger->logWhite(L"Type 'Enter' to finish the sample");
				});
			}
		});
		logger->logGreen(L"Done");

		logger->logWhite(L"Connect to scene");
		return scene->connect().then([nbMsgToSend]() {
			logger->logGreen(L"Done");
			for (int i = 0; i < nbMsgToSend; i++)
			{
				scene->sendPacket(L"echo.in", [](bytestream* stream) {
					int32 number1(rand()), number2(rand()), number3(rand());
					*stream << number1 << number2 << number3;
					logger->logWhite(L"Sending message: [ " + to_wstring(number1) + L" ; " + to_wstring(number2) + L" ; " + to_wstring(number3) + L" ]");
				});
			}
		});
	});
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	logger->logWhite(L"Type 'Enter' to start the sample");
	cin.ignore();

	logger->logWhite(L"Create client");
	Configuration config(L"997bc6ac-9021-2ad6-139b-da63edee8c58", L"echo");
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
