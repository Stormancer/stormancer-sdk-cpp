#include <stormancer.h>

using namespace Stormancer;

int main(int argc, char* argv[])
{
	std::set_terminate([]() {
		bool termin = true;
	});

	wcout << L"Starting echo..." << endl;

	ClientConfiguration config(L"test", L"echo");
	config.serverEndpoint = L"http://localhost:8081";

	Client client(&config);
	auto task = client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		Scene* scene = t.get();
		scene->addRoute(L"echo.out", [](Packet<IScenePeer>* p) {
			string str;
			*p->stream >> str;
			wstring str2 = Helpers::to_wstring(str);
			wcout << str2 << endl;
		});

		return scene->connect().then([scene](pplx::task<void> t2) {
			if (t2.is_done())
			{
				wcout << L"Connected to scene!" << endl;
				scene->sendPacket(L"echo.in", [](bytestream* stream) {
					*stream << "hello";
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
