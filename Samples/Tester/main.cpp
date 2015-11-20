#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
bool stop = false;
const char* accountId = "997bc6ac-9021-2ad6-139b-da63edee8c58";
const char* applicationName = "tester";
const char* sceneName = "main";
Configuration* config = nullptr;
Client* client = nullptr;
Scene* sceneMain;
pplx::task<void> syncclockTask;

std::deque<std::function<void()>> tests;

void execNextTest()
{
	if (tests.size())
	{
		auto test = tests[0];
		pplx::task<void>(test).then([](pplx::task<void> t) {
			try
			{
				t.wait();
			}
			catch (const std::exception& ex)
			{
				throw ex;
			}
		});
		tests.pop_front();
	}
}

void test_echo_received(Packetisp_ptr p)
{
	std::string message;
	*p->stream >> message;
	logger->log(LogLevel::Debug, "test_echo_received", ("Message received (" + message + ")").c_str(), "");
	if (message == "stormancer")
	{
		logger->log(LogLevel::Info, "test_echo_received", "Echo OK", "");
		execNextTest();
	}
}

pplx::task<void> test_rpc_client(RpcRequestContext_ptr rc)
{
	std::string message;
	*rc->inputStream() >> message;
	logger->log(LogLevel::Debug, "test_rpc_client", ("rpc request received (" + message + ")").c_str(), "");

	rc->cancellationToken().register_callback([]() {
		logger->log(LogLevel::Debug, "test_rpc_client", "test_rpc_client: rpc request cancelled", "");
		logger->log(LogLevel::Info, "test_rpc_client", "RPC on client cancel OK", "");
		execNextTest();
	});

	return pplx::task<void>([message, rc]() {
		Sleep(1000);
		if (!rc->cancellationToken().is_canceled())
		{
			if (message == "stormancer")
			{
				logger->log(LogLevel::Info, "test_rpc_client", "RPC on client OK", "");
				logger->log(LogLevel::Debug, "test_rpc_client", "sending rpc response", "");
				rc->sendValue(Action<bytestream*>([message](bytestream* bs) {
					*bs << message;
				}), PacketPriority::MEDIUM_PRIORITY);
				execNextTest();
			}
			else
			{
				logger->log(LogLevel::Error, "test_rpc_client", "rpc server failed", "");
				throw std::runtime_error("bad rpc server request (bad message)");
			}
		}
	});
}

void test_rpc_server_cancelled(Packetisp_ptr p)
{
	logger->log(LogLevel::Info, "test_rpc_server_cancelled", "RPC on server cancel OK", "");
}

void test_connect()
{
	logger->log(LogLevel::Debug, "test_connect", "Create client", "");
	config = Configuration::forAccount(accountId, applicationName);
	client = Client::createClient(config);
	logger->log(LogLevel::Info, "test_connect", "Create client OK", "");

	logger->log(LogLevel::Debug, "test_connect", "Get scene", "");
	client->getPublicScene(sceneName).then([](Scene* scene) {
		sceneMain = scene;
		logger->log(LogLevel::Info, "test_connect", "Get scene OK", "");

		logger->log(LogLevel::Debug, "test_connect", "Add route", "");
		scene->addRoute("echo", test_echo_received);
		scene->addRoute("rpcservercancelled", test_rpc_server_cancelled);
		logger->log(LogLevel::Info, "test_connect", "Add route OK", "");

		logger->log(LogLevel::Debug, "test_connect", "Add procedure", "");
		scene->dependencyResolver()->resolve<IRpcService>()->addProcedure("rpc", test_rpc_client, true);
		logger->log(LogLevel::Info, "test_connect", "Add procedure OK", "");

		logger->log(LogLevel::Debug, "test_connect", "Connect to scene", "");
		return scene->connect().then([](pplx::task<void> t) {
			try
			{
				t.wait();
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Error, "test_connect", "Failed to connect to the scene", "");
				throw ex;
			}
			logger->log(LogLevel::Info, "test_connect", "Connect OK", "");
			execNextTest();
		});
	});
}

void test_echo()
{
	logger->log(LogLevel::Debug, "test_echo", "test_echo", "");
	try
	{
		sceneMain->sendPacket("echo", [](bytestream* stream) {
			*stream << "stormancer";
			logger->log(LogLevel::Debug, "test_echo", "Sending message...", "");
		});
	}
	catch (const std::exception& ex)
	{
		logger->log(LogLevel::Error, "test_echo", (std::string("Can't send data to the scene through the 'echo' route.\n") + ex.what()).c_str(), "");
		throw ex;
	}
}

void test_rpc_server()
{
	logger->log(LogLevel::Debug, "test_rpc_server", "RPC on server", "");
	sceneMain->dependencyResolver()->resolve<IRpcService>()->rpc("rpc", Action<bytestream*>([](bytestream* stream) {
		*stream << "stormancer";
	}), PacketPriority::MEDIUM_PRIORITY)->subscribe([](Packetisp_ptr packet) {
		std::string message;
		*packet->stream >> message;
		logger->log(LogLevel::Debug, "test_rpc_server", ("rpc response received (" + message + ")").c_str(), "");
		if (message == "stormancer")
		{
			logger->log(LogLevel::Info, "test_rpc_server", "RPC on server OK", "");
		}

		//execNextTest(); // don't do that, the server send back a rpc for the next test!
	});
}

void test_rpc_server_cancel()
{
	logger->log(LogLevel::Debug, "test_rpc_server_cancel", "Rpc on server cancel", "");
	auto subscription = sceneMain->dependencyResolver()->resolve<IRpcService>()->rpc("rpc", Action<bytestream*>([](bytestream* stream) {
		*stream << "stormancer";
	}), PacketPriority::MEDIUM_PRIORITY)->subscribe([](Packetisp_ptr packet) {
		logger->log(LogLevel::Debug, "test_rpc_server_cancel", "rpc response received, but this RPC should be cancelled.", "");
	});
	subscription->unsubscribe();
}

void test_syncclock()
{
	logger->log(LogLevel::Debug, "test_syncclock", "test sync clock", "");
	stop = false;
	syncclockTask = pplx::create_task([]() {
		while (!client->lastPing() && !stop)
		{
			Sleep(100);
		}
		if (client->lastPing())
		{
			int64 clock = client->clock();
			if (clock)
			{
				logger->log(LogLevel::Debug, "test_syncclock", ("clock: " + to_string(clock / 1000.0)).c_str(), "");
				logger->log(LogLevel::Info, "test_syncclock", "SyncClock OK", "");
				execNextTest();
			}
		}
	});
}

void test_disconnect()
{
	logger->log(LogLevel::Debug, "test_disconnect", "test disconnect", "");
	sceneMain->disconnect().then([](pplx::task<void> t)
	{
		try
		{
			t.wait();
		}
		catch (const std::exception& ex)
		{
			logger->log(LogLevel::Error, "test_disconnect", "Failed to connect to the scene", "");
			throw ex;
		}
		logger->log(LogLevel::Info, "test_disconnect", "Disconnect OK", "");
		execNextTest();
	});
}

void test_steam()
{
	logger->log(LogLevel::Debug, "test_steam", "Authentication", "");

	auto authService = sceneMain->dependencyResolver()->resolve<IAuthenticationService>();
	authService->steamLogin("TEST").then([](Scene* scene) {
		logger->log(LogLevel::Debug, "test_steam", "Authentication OK", "");
		execNextTest();
	});
}

void clean()
{
	logger->log(LogLevel::Debug, "clean", "deleting scene and client", "");
	stop = true;
	sceneMain->destroy();
	sceneMain = nullptr;
	client->destroy();
	client = nullptr;
	config->destroy();
	config = nullptr;
	logger->log(LogLevel::Debug, "clean", "scene and client deleted", "");

	execNextTest();
}

void the_end()
{
	logger->log(LogLevel::Info, "clean", "TESTS SUCCESSFUL !", "");

	execNextTest();
}

int main(int argc, char* argv[])
{
	tests.push_back(test_connect);
	tests.push_back(test_echo);
	tests.push_back(test_rpc_server);
	tests.push_back(test_rpc_server_cancel);
	tests.push_back(test_syncclock);
	tests.push_back(test_disconnect);
	tests.push_back(clean);

	//tests.push_back(test_connect);
	//tests.push_back(test_echo);
	//tests.push_back(test_rpc_server);
	//tests.push_back(test_rpc_server_cancel);
	//tests.push_back(test_syncclock);
	//tests.push_back(test_disconnect);
	//tests.push_back(clean);

	tests.push_back(the_end);

	execNextTest();

	cin.ignore();
	stop = true;

	return 0;
}
