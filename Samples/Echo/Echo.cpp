#include <stormancer.h>

using namespace Stormancer;

int main(int argc, char* argv[])
{
	std::set_terminate([]() {
		int termin = 1;
	});

	wcout << L"Starting echo..." << endl;

	ClientConfiguration config(L"905f108e-18bc-0d56-45c2-0907de336e65", L"test");
	config.serverEndpoint = L"http://localhost:8888";

	Client client(&config);
	auto task = client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		Scene* scene = t.get();
		scene->addRoute(L"echo.out", [](Packet<IScenePeer>* p) {
			wstring str;
			ISerializable::deserialize(p->stream, str);
			wcout << str << endl;
		});

		return scene->connect().then([&scene](pplx::task<void> t2) {
			if (t2.is_done())
			{
				scene->sendPacket(L"echo.in", [](bytestream* stream) {
					*stream << L"hello";
				});
			}
			else
			{
				wcout << L"Bad stuff happened..." << endl;
			}
		});
	});

	try
	{
		task.wait();
	}
	catch (const exception &e)
	{
		wcout << L"Error exception:\n" << e.what();
	}

	cin.ignore();

	return 0;
}
