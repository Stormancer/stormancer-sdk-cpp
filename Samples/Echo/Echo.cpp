#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;

int main(int argc, char* argv[])
{
	std::set_terminate([]() {
		bool termin = true;
	});

	cout << "Starting echo..." << endl;
	cout.flush();

	ILogger::instance(new ConsoleLogger);
	//ILogger::instance(new FileLogger);

	ClientConfiguration config(L"test", L"echo");
	config.serverEndpoint = L"http://localhost:8081";

	Client client(&config);
	auto task = client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		Scene* scene = t.get();
		scene->addRoute(L"echo.out", [scene](Packet<IScenePeer>* p) {
			string message;
			*p->stream >> message;
			cout << "Received message: " << message << endl;
			cout.flush();

			if (message == "hello")
			{
				cout << "Test successful. Disconnecting..." << endl;
				cout.flush();
				scene->disconnect();
			}
		});

		return scene->connect().then([scene](pplx::task<void> t2) {
			if (t2.is_done())
			{
				cout << "Connected to scene!" << endl;
				cout.flush();
				scene->sendPacket(L"echo.in", [](bytestream* stream) {
					string message = "hello";
					cout << "Sending message: " << message << endl;
					cout.flush();
					*stream << message;
				});
			}
			else
			{
				cout << "Bad stuff happened..." << endl;
				cout.flush();
			}
		});
	});

	try
	{
		task.wait();
	}
	catch (const exception &e)
	{
		cout << "Error exception:\n" << e.what();
		cout.flush();
	}

	cin.ignore();

	return 0;
}
