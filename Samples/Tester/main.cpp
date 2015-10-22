#include <stormancer.h>
#include "ConsoleLogger.h"

using namespace Stormancer;
using namespace std;

auto logger = (ConsoleLogger*)ILogger::instance(new ConsoleLogger(Stormancer::LogLevel::Trace));
Configuration* config = nullptr;
Client* client = nullptr;
Scene_ptr scene;
bool stop = false;
const std::string stormancer = "stormancer";
const std::string accountId = "997bc6ac-9021-2ad6-139b-da63edee8c58";
//const std::string accountId = "test";
const std::string applicationName = "tester";
const std::string sceneName = "main";
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
				//
			}
		});
		tests.pop_front();
	}
}

void test_echo_received(Packetisp_ptr p)
{
	std::string message;
	*p->stream >> message;
	logger->logWhite("Message received (" + message + ")");
	if (message == stormancer)
	{
		logger->logGreen("Echo OK");
		execNextTest();
	}
}

pplx::task<void> test_rpc_client(RpcRequestContex_ptr rc)
{
	std::string message;
	*rc->inputStream() >> message;
	logger->logWhite("rpc request received (" + message + ")");

	rc->cancellationToken().register_callback([]() {
		logger->logWhite("test_rpc_client: rpc request cancelled");
		logger->logGreen("RPC on client cancel OK");
		execNextTest();
	});

	return pplx::task<void>([message, rc]() {
		Sleep(1000);
		if (!rc->cancellationToken().is_canceled())
		{
			if (message == stormancer)
			{
				logger->logGreen("RPC on client OK");
				logger->logWhite("sending rpc response");
				rc->sendValue(Action<bytestream*>([message](bytestream* bs) {
					*bs << message;
				}), PacketPriority::MEDIUM_PRIORITY);
				execNextTest();
			}
			else
			{
				logger->logRed("rpc server failed");
				throw std::runtime_error("bad rpc server request (bad message)");
			}
		}
	});
}

void test_rpc_server_cancelled(Packetisp_ptr p)
{
	logger->logWhite("RPC on server has been cancelled");
	logger->logGreen("RPC on server cancel OK");
}

void test_connect()
{
	logger->logWhite("Create client");
	config = new Configuration(accountId, applicationName);
	//config.serverEndpoint = "http://localhost:8081";
	client = new Client(config);
	logger->logGreen("Create client OK");

	logger->logWhite("Get scene");
	client->getPublicScene(sceneName).then([](Scene_ptr sc) {
		scene = sc;
		logger->logGreen("Get scene OK");

		logger->logWhite("Add route");
		scene->addRoute("echo", test_echo_received);
		scene->addRoute("rpcservercancelled", test_rpc_server_cancelled);
		logger->logGreen("Add route OK");

		logger->logWhite("Add procedure");
		((RpcService*)scene->getComponent("rpcService"))->addProcedure("rpc", test_rpc_client, true);
		logger->logGreen("Add procedure OK");

		logger->logWhite("Connect to scene");
		return scene->connect().then([](pplx::task<void> t) {
			try
			{
				t.wait();
			}
			catch (const std::exception& ex)
			{
				//
			}
			logger->logGreen("Connect OK");
			execNextTest();
		});
	});
}

void test_echo()
{
	try
	{
		scene->sendPacket("echo", [](bytestream* stream) {
			*stream << stormancer;
			logger->logWhite("Sending message...");
		});
	}
	catch (const std::exception& e)
	{
		logger->logRed(std::string("Can't send data to the scene through the 'echo' route.\n") + e.what());
	}
}

void test_rpc_server()
{
	logger->logWhite("sending rpc request");
	((RpcService*)scene->getComponent("rpcService"))->rpc("rpc", Action<bytestream*>([](bytestream* stream) {
		*stream << stormancer;
	}), PacketPriority::MEDIUM_PRIORITY).subscribe([](Packetisp_ptr packet) {
		std::string message;
		*packet->stream >> message;
		logger->logWhite("rpc response received (" + message + ")");
		if (message == stormancer)
		{
			logger->logGreen("RPC on server OK");
		}
		// execNextTest(); // don't do that, the server send back a rpc for the next test!
	});
}

void test_rpc_server_cancel()
{
	logger->logWhite("sending rpc request");
	auto subscription = ((RpcService*)scene->getComponent("rpcService"))->rpc("rpc", Action<bytestream*>([](bytestream* stream) {
		*stream << stormancer;
	}), PacketPriority::MEDIUM_PRIORITY).subscribe([](Packetisp_ptr packet) {
		logger->logRed("rpc response received, but this RPC should be cancelled.");
	});
	subscription.unsubscribe();
}

void test_syncclock()
{
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
				logger->logYellow("clock: " + to_string(clock / 1000.0));
				logger->logGreen("SyncClock OK");
				execNextTest();
			}
		}
	});
}

void test_disconnect()
{
	scene->disconnect().then([](pplx::task<void> t)
	{
		try
		{
			t.wait();

			logger->logGreen("Disconnect OK");
			execNextTest();
		}
		catch (const std::exception& ex)
		{
			//
		}
	});
}

int main(int argc, char* argv[])
{
	srand((uint32)time(NULL));

	tests.push_back(test_connect);
	tests.push_back(test_echo);
	tests.push_back(test_rpc_server);
	tests.push_back(test_rpc_server_cancel);
	tests.push_back(test_syncclock);
	tests.push_back(test_disconnect);

	execNextTest();

	cin.ignore();
	stop = true;
	syncclockTask.wait();

	return 0;
}
