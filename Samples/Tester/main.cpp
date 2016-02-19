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
Scene* sceneMain = nullptr;
pplx::task<void> syncclockTask;

std::deque<std::function<void()>> tests;

void execNextTest()
{
	if (tests.size())
	{
		auto test = tests[0];
		pplx::create_task(test);
		tests.pop_front();
	}
}

std::string connectionStateToString(ConnectionState connectionState)
{
	std::string stateStr = std::to_string((int)connectionState) + " ";
	switch (connectionState)
	{
	case ConnectionState::Disconnected:
		stateStr += "Disconnected";
		break;
	case ConnectionState::Connecting:
		stateStr += "Connecting";
		break;
	case ConnectionState::Connected:
		stateStr += "Connected";
		break;
	case ConnectionState::Disconnecting:
		stateStr += "Disconnecting";
		break;
	}
	return stateStr;
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
				rc->sendValue([message](bytestream* bs) {
					*bs << message;
				}, PacketPriority::MEDIUM_PRIORITY);
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
	try
	{
		logger->log(LogLevel::Debug, "test_connect", "Create client", "");
		config = Configuration::forAccount(accountId, applicationName);
		client = Client::createClient(config);
		logger->log(LogLevel::Info, "test_connect", "Create client OK", "");

		client->onConnectionStateChanged([](ConnectionState state) {
			try
			{
				auto stateStr = connectionStateToString(state);
				logger->log(LogLevel::Info, "Client", "Client connection state changed", stateStr.c_str());
				if (state == ConnectionState::Disconnected)
				{
					Stormancer::destroy(client);
				}
			}
			catch (const std::exception& ex)
			{
				logger->log(ex);
			}
		});

		logger->log(LogLevel::Debug, "test_connect", "Get scene", "");
		client->getPublicScene(sceneName).then([](Result<Scene*>* result) {
			try
			{
				if (result->success())
				{
					sceneMain = result->get();

					sceneMain->onConnectionStateChanged([](ConnectionState state) {
						auto stateStr = connectionStateToString(state);
						logger->log(LogLevel::Info, "Scene", "Scene connection state changed", stateStr.c_str());
						if (state == ConnectionState::Disconnected)
						{
							Stormancer::destroy(sceneMain);
						}
					});

					logger->log(LogLevel::Info, "test_connect", "Get scene OK", "");

					logger->log(LogLevel::Debug, "test_connect", "Add route", "");
					sceneMain->addRoute("echo", test_echo_received);
					sceneMain->addRoute("rpcservercancelled", test_rpc_server_cancelled);
					logger->log(LogLevel::Info, "test_connect", "Add route OK", "");

					logger->log(LogLevel::Debug, "test_connect", "Add procedure", "");
					sceneMain->dependencyResolver()->resolve<IRpcService>()->addProcedure("rpc", test_rpc_client, true);
					logger->log(LogLevel::Info, "test_connect", "Add procedure OK", "");

					logger->log(LogLevel::Debug, "test_connect", "Connect to scene", "");
					sceneMain->connect().then([](Result<>* result2) {
						if (result2->success())
						{
							logger->log(LogLevel::Info, "test_connect", "Connect OK", "");
							logger->log(LogLevel::Warn, "test_connect", "Waiting 2000 ms...", "");
							pplx::create_task([]() {
								Sleep(2000);
							}).wait();
							execNextTest();
						}
						else
						{
							logger->log(LogLevel::Error, "test_connect", "Failed to connect to the scene", result2->reason());
						}
						destroy(result2);
					});
				}
				else
				{
					logger->log(LogLevel::Error, "test_connect", "Failed to get the scene", result->reason());
				}
				destroy(result);
			}
			catch (const std::exception& ex)
			{
				logger->log(ex);
			}
		});
	}
	catch (const std::exception& ex)
	{
		logger->log(ex);
	}
}

void test_echo()
{
	logger->log(LogLevel::Debug, "test_echo", "test_echo", "");
	auto result = sceneMain->sendPacket("echo", [](bytestream* stream) {
		*stream << "stormancer";
		logger->log(LogLevel::Debug, "test_echo", "Sending message...", "");
	});
	if (!result->success())
	{
		logger->log(LogLevel::Error, "test_echo", "Can't send data to the scene through the 'echo' route.", result->reason());
	}
}

void test_rpc_server()
{
	logger->log(LogLevel::Debug, "test_rpc_server", "RPC on server", "");
	sceneMain->dependencyResolver()->resolve<IRpcService>()->rpc("rpc", std::function<void(bytestream*)>([](bytestream* stream) {
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

	// Test disconnecting while doing an RPC ! =)
	//client->disconnect().then([](Result<>* result) {
	//	if (result->success())
	//	{
	//		logger->log(LogLevel::Info, "client.disconnect", "Disconnect OK", "");
	//	}
	//	else
	//	{
	//		logger->log(LogLevel::Error, "client.disconnect", "Disconnect failed", result->reason());
	//	}
	//});
}

void test_rpc_server_cancel()
{
	logger->log(LogLevel::Debug, "test_rpc_server_cancel", "Rpc on server cancel", "");
	auto subscription = sceneMain->dependencyResolver()->resolve<IRpcService>()->rpc("rpc", std::function<void(bytestream*)>([](bytestream* stream) {
		*stream << "stormancer";
	}), PacketPriority::MEDIUM_PRIORITY)->subscribe([](Packetisp_ptr packet) {
		logger->log(LogLevel::Debug, "test_rpc_server_cancel", "rpc response received, but this RPC should be cancelled.", "");
	});
	auto sub = subscription.lock();
	if (sub)
	{
		sub->unsubscribe();
	}
	else
	{
		logger->log(LogLevel::Error, "test_rpc_server_cancel", "subscription is empty", "");
	}
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
	client->disconnect().then([](Result<>* result2) {
		if (result2->success())
		{
			logger->log(LogLevel::Info, "test_disconnect", "Disconnect client OK", "");
			execNextTest();
		}
		else
		{
			logger->log(LogLevel::Error, "test_disconnect", "Failed to disconnect client", result2->reason());
		}
		destroy(result2);
	});
}

void clean()
{
	logger->log(LogLevel::Debug, "clean", "deleting scene and client", "");
	stop = true;
	destroy(sceneMain);
	sceneMain = nullptr;
	destroy(client);
	client = nullptr;
	destroy(config);
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
	//tests.push_back(clean);

	tests.push_back(the_end);

	execNextTest();

	cin.ignore();
	stop = true;

	return 0;
}
