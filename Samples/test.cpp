#include "pch.h"
#include "test.h"




using namespace std;
using namespace Stormancer;

namespace
{



	auto logger = make_shared<ConsoleLogger>();

	bool stop = false;
	const string endpoint = "http://api.stormancer.com/";
	const string accountId = "58ec9ba7-56e4-3d89-2c55-c9435e08b26b";
	//const string endpoint = "http://localhost:8081/";
	//const string accountId = "test";
	const string applicationName = "tester";
	const string sceneName = "main";
	Configuration_ptr config;
	Client_ptr client;
	Scene_ptr sceneMain;

	deque<function<void()>> tests;
    bool testsDone = false;
    bool testsPassed = false;

	void execNextTest()
	{
		if (!tests.empty())
		{
			auto test = tests[0];
			tests.pop_front();
			pplx::create_task(test);
		}
		else
		{
			logger->log(LogLevel::Info, "execNextTest", "TESTS SUCCESSFUL !");
            testsDone = true;
		}
	}

	string connectionStateToString(ConnectionState connectionState)
	{
		string stateStr = to_string((int)connectionState) + " ";
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

	void onMessage(Packetisp_ptr p)
	{
		string buffer;
		*(p->stream) >> buffer;
		msgpack::unpacked unp;
		msgpack::unpack(unp, buffer.data(), buffer.size());
		string message;
		unp.get().convert(&message);

		logger->log(LogLevel::Debug, "onMessage", "Message received", message);

		if (message == "echo")
		{
			logger->log(LogLevel::Debug, "onMessage", "ECHO OK");
			execNextTest();
		}
		else if (message == "rpcServer_CancelOk")
		{
			logger->log(LogLevel::Debug, "onMessage", "RPC SERVER CANCEL OK");
			execNextTest();
		}
		else if (message == "rpcClient_ExceptionOk")
		{
			logger->log(LogLevel::Debug, "onMessage", "RPC CLIENT EXCEPTION OK");
			execNextTest();
		}
	}

	pplx::task<void> test_rpc_client_received(RpcRequestContext_ptr rc)
	{
		string buffer;
		*(rc->inputStream()) >> buffer;
		msgpack::unpacked unp;
		msgpack::unpack(unp, buffer.data(), buffer.size());
		string message;
		unp.get().convert(&message);

		logger->log(LogLevel::Debug, "test_rpc_client", "RPC request received", message);

		if (message == "stormancer")
		{
			logger->log(LogLevel::Debug, "test_rpc_client", "RPC CLIENT OK");
			logger->log(LogLevel::Debug, "test_rpc_client", "sending rpc response...");
			rc->sendValue([message](bytestream* bs) {
				msgpack::pack(bs, message);
			});
			execNextTest();
			return pplx::task_from_result();
		}
		else
		{
			logger->log(LogLevel::Error, "test_rpc_client", "RPC server failed", "Bad message");
			throw runtime_error("RPC server failed (bad message)");
		}
	}

	pplx::task<void> test_rpc_client_cancel_received(RpcRequestContext_ptr rc)
	{
		rc->cancellationToken().register_callback([]() {
			logger->log(LogLevel::Debug, "test_rpc_client", "RPC CLIENT CANCEL OK");
			execNextTest();
		});

		string buffer;
		*(rc->inputStream()) >> buffer;
		msgpack::unpacked unp;
		msgpack::unpack(unp, buffer.data(), buffer.size());
		string message;
		unp.get().convert(&message);

		logger->log(LogLevel::Debug, "test_rpc_client", "RPC request received", message);

		Stormancer::taskDelay(std::chrono::milliseconds(1000), rc->cancellationToken()).wait();

		if (!rc->cancellationToken().is_canceled())
		{
			logger->log(LogLevel::Error, "test_rpc_client", "RPC server should be cancelled");
			throw runtime_error("RPC server should be cancelled");
		}

		return pplx::task_from_result();
	}

	pplx::task<void> test_rpc_client_exception_received(RpcRequestContext_ptr rc)
	{
		throw std::runtime_error("An excepion occured!");
	}

	void test_rpc_server_cancelled(Packetisp_ptr p)
	{
		logger->log(LogLevel::Debug, "test_rpc_server_cancelled", "RPC on server cancel OK");
	}

	void test_connect()
	{
		logger->log(LogLevel::Info, "test_connect", "CONNECT");

		try
		{
			client->getConnectionStateChangedObservable().subscribe([](ConnectionState state) {
				try
				{
					auto stateStr = connectionStateToString(state);
					logger->log(LogLevel::Debug, "test_connect", "Client connection state changed", stateStr.c_str());

					// Test: delete the client in connection state changed event
					if (state == ConnectionState::Disconnected)
					{
						client.reset();
						config.reset();
					}
				}
				catch (const exception& ex)
				{
					logger->log(ex);
				}
			});

			logger->log(LogLevel::Debug, "test_connect", "Get scene");

			client->connectToPublicScene(sceneName, [](Scene_ptr scene) {
				logger->log(LogLevel::Debug, "test_connect", "Get scene OK");

				scene->getConnectionStateChangedObservable().subscribe([scene](ConnectionState state) {
					auto stateStr = connectionStateToString(state);
					logger->log(LogLevel::Debug, "test_connect", "Scene connection state changed", stateStr.c_str());
				});

				logger->log(LogLevel::Debug, "test_connect", "Add route");
				scene->addRoute("message", onMessage);
				logger->log(LogLevel::Debug, "test_connect", "Add route OK");

				logger->log(LogLevel::Debug, "test_connect", "Add procedure");
				auto rpcService = scene->dependencyResolver()->resolve<RpcService>();
				rpcService->addProcedure("rpc", test_rpc_client_received, Stormancer::MessageOriginFilter::Host, true);
				rpcService->addProcedure("rpcCancel", test_rpc_client_cancel_received, Stormancer::MessageOriginFilter::Host, true);
				rpcService->addProcedure("rpcException", test_rpc_client_exception_received, Stormancer::MessageOriginFilter::Host, true);
				logger->log(LogLevel::Debug, "test_connect", "Add procedure OK");

				logger->log(LogLevel::Debug, "test_connect", "Connect to scene");
			}).then([](pplx::task<Scene_ptr> task) {
				try
				{
					sceneMain = task.get();

					logger->log(LogLevel::Debug, "test_connect", "Connect to scene OK");
					if (sceneMain)
					{
						logger->log(LogLevel::Debug, "test_connect", "External addresses: " + sceneMain->hostConnection()->metadata("externalAddrs"));
					}
					execNextTest();
				}
				catch (const exception& ex)
				{
					logger->log(LogLevel::Error, "test_connect", "Failed to get and connect the scene.", ex.what());
				}
			});
		}
		catch (const exception& ex)
		{
			logger->log(ex);
		}
	}

	void test_echo()
	{
		logger->log(LogLevel::Info, "test_echo", "ECHO");

		try
		{
			if (sceneMain)
			{
				logger->log(LogLevel::Debug, "test_echo", "Sending message...");
				sceneMain->send("message", [](bytestream* stream) {
					msgpack::pack(stream, "echo");
				});
			}
		}
		catch (exception& ex)
		{
			logger->log(LogLevel::Error, "test_echo", "Can't send data to the scene.", ex.what());
		}
	}

	void test_rpc_server()
	{
		logger->log(LogLevel::Info, "test_rpc_server", "RPC SERVER");

		if (sceneMain)
		{
			// get the RPC service
			auto rpcService = sceneMain->dependencyResolver()->resolve<RpcService>();

			//int i = 123;
			//rpcService->rpc<void>("rpc2").then([]() { std::cout << "void 0" << std::endl; } );
			//rpcService->rpc<void>("rpc2", i).then([]() { std::cout << "void 1" << std::endl; } );
			//rpcService->rpc<void>("rpc2", i, i).then([]() { std::cout << "void 2" << std::endl; } );
			//rpcService->rpc<void>("rpc2", i, i, i).then([]() { std::cout << "void 3" << std::endl; } );
			//rpcService->rpc<int>("rpc2").then([](int j) { std::cout << "int 0" << j << std::endl; } );
			//rpcService->rpc<int>("rpc2", i).then([](int j) { std::cout << "int 1" << j << std::endl; } );
			//rpcService->rpc<int>("rpc2", i, i).then([](int j) { std::cout << "int 2" << j << std::endl; } );
			//rpcService->rpc<int>("rpc2", i, i, i).then([](int j) { std::cout << "int 3" << j << std::endl; } );

			//rpcService->rpcWriter("rpc2", [](Stormancer::bytestream* stream) {
			//	msgpack(stream, 123);
			//});

			//rpcService->rpcWriter<int>("rpc2", [](Stormancer::bytestream* stream) {
			//	(*stream) << 123;
			//}, [](Packetisp_ptr p) {
			//	int i;
			//	*p->stream >> i;
			//	return i;
			//});

			// We do an RPC on the server (by sending the string "stormancer") and get back a string response (that should be "stormancer" too)
			rpcService->rpc<string>("rpc", "stormancer").then([](pplx::task<string> t) {
				try
				{
					string response = t.get();

					logger->log(LogLevel::Debug, "test_rpc_server", "rpc response received", response.c_str());

					if (response == "stormancer")
					{
						logger->log(LogLevel::Debug, "test_rpc_server", "RPC SERVER OK");
					}
					else
					{
						logger->log(LogLevel::Error, "test_rpc_server", "RPC SERVER FAILED", "Bad RPC response");
					}
					execNextTest();
				}
				catch (const exception& ex)
				{
					logger->log(LogLevel::Error, "test_rpc_server", "RPC SERVER failed", ex.what());
				}
			});

			// Test disconnecting while doing an RPC ! =)
			//client->disconnect().then([](pplx::task<void> t) {
			//	try
			//	{
			//		t.get();
			//		logger->log(LogLevel::Info, "test_rpc_server", "Disconnect OK");
			//	}
			//	catch (std::exception ex)
			//	{
			//		logger->log(LogLevel::Error, "test_rpc_server", "Disconnect failed", ex.what());
			//	}
			//});
		}
	}

	void test_rpc_server_cancel()
	{
		logger->log(LogLevel::Info, "test_rpc_server_cancel", "RPC SERVER CANCEL");

		if (sceneMain)
		{
			auto rpcService = sceneMain->dependencyResolver()->resolve<RpcService>();

			auto subscription = rpcService->rpc_observable("rpcCancel", [](bytestream* stream) {
				msgpack::pack(stream, "stormancer2");
			}).subscribe([](Packetisp_ptr packet) {
				logger->log(LogLevel::Warn, "test_rpc_server_cancel", "rpc response received, but this RPC should be cancelled.");
			}, [](std::exception_ptr exp) {
				try {
					std::rethrow_exception(exp);
				}
				catch (std::exception& ex) {
					logger->log(LogLevel::Error, "test_rpc_server_cancel", "Rpc failed", ex.what());
				}
			});

			pplx::create_task([subscription]() {
				this_thread::sleep_for(chrono::milliseconds(1));
				if (subscription.is_subscribed())
				{
					subscription.unsubscribe();
				}
				else
				{
					logger->log(LogLevel::Error, "test_rpc_server_cancel", "subscription is empty");
				}
			});
		}
	}

	void test_rpc_server_exception()
	{
		logger->log(LogLevel::Info, "test_rpc_server_exception", "RPC SERVER EXCEPTION");

		if (sceneMain)
		{
			auto rpcService = sceneMain->dependencyResolver()->resolve<RpcService>();

			rpcService->rpc<void>("rpcException").then([](pplx::task<void> t) {
				try
				{
					t.get();
					logger->log(LogLevel::Error, "test_rpc_server_exception", "Rpc should fail");
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Debug, "test_rpc_server_exception", "RPC SERVER EXCEPTION OK", ex.what());
					execNextTest();
				}
			});
		}
	}

	void test_rpc_server_clientException()
	{
		logger->log(LogLevel::Info, "test_rpc_server_clientException", "RPC SERVER CLIENT EXCEPTION");

		if (sceneMain)
		{
			auto rpcService = sceneMain->dependencyResolver()->resolve<RpcService>();

			rpcService->rpc<void>("rpcClientException").then([](pplx::task<void> t) {
				try
				{
					t.get();
					logger->log(LogLevel::Error, "test_rpc_server_clientException", "Rpc should fail");
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Debug, "test_rpc_server_clientException", "RPC SERVER CLIENT EXCEPTION OK", ex.what());
					execNextTest();
				}
			});
		}
	}

	void test_rpc_client()
	{
		logger->log(LogLevel::Info, "test_rpc_client", "RPC CLIENT");

		try
		{
			if (sceneMain)
			{
				sceneMain->sendPacket("message", [](bytestream* stream) {
					msgpack::pack(stream, "rpc");
				});
			}
		}
		catch (exception& ex)
		{
			logger->log(LogLevel::Error, "test_rpc_client", "Can't send data to the scene.", ex.what());
		}
	}

	void test_rpc_client_cancel()
	{
		logger->log(LogLevel::Info, "test_rpc_client_cancel", "RPC CLIENT CANCEL");

		try
		{
			if (sceneMain)
			{
				sceneMain->sendPacket("message", [](bytestream* stream) {
					msgpack::pack(stream, "rpcCancel");
				});
			}
		}
		catch (exception& ex)
		{
			logger->log(LogLevel::Error, "test_rpc_client_cancel", "Can't send data to the scene.", ex.what());
		}
	}

	void test_rpc_client_exception()
	{
		logger->log(LogLevel::Info, "test_rpc_client_exception", "RPC CLIENT EXCEPTION");

		try
		{
			if (sceneMain)
			{
				sceneMain->sendPacket("message", [](bytestream* stream) {
					msgpack::pack(stream, "rpcException");
				});
			}
		}
		catch (exception& ex)
		{
			logger->log(LogLevel::Error, "test_rpc_client_exception", "Can't send data to the scene.", ex.what());
		}
	}

	void test_syncclock()
	{
		logger->log(LogLevel::Info, "test_syncclock", "SYNC CLOCK");

		stop = false;
		pplx::create_task([]() {
			while (!stop && !client->lastPing())
			{
				this_thread::sleep_for(chrono::milliseconds(100));
			}
			if (!stop && client->lastPing() > 0)
			{
				int64 clock = client->clock();
				if (clock)
				{
					std::string clockStr = to_string(clock / 1000.0);
					logger->log(LogLevel::Debug, "test_syncclock", "clock", clockStr.c_str());
					logger->log(LogLevel::Debug, "test_syncclock", "SyncClock OK");
					execNextTest();
				}
			}
		});
	}

	void test_disconnect()
	{
		logger->log(LogLevel::Info, "test_disconnect", "DISCONNECT");

		client->disconnect().then([](pplx::task<void> t) {
			try
			{
				logger->log(LogLevel::Debug, "test_disconnect", "Disconnect client OK");
				execNextTest();
			}
			catch (exception& ex)
			{
				logger->log(LogLevel::Error, "test_disconnect", "Failed to disconnect client", ex.what());
			}
		});
	}

	void test_clean()
	{
		logger->log(LogLevel::Info, "test_clean", "CLEAN");

		try
		{
			stop = true;
			sceneMain.reset();
			client.reset();
			config.reset();
			logger->log(LogLevel::Debug, "test_clean", "scene and client deleted");
            
            testsPassed = true;
		}
		catch (std::exception& ex)
		{
			logger->log(LogLevel::Error, "test_clean", "exception: ", ex.what());
		}

		execNextTest();
	}
} // namespace

void run_all_tests()
{
    run_all_tests_nonblocking();
	
	cin.ignore();
	stop = true;
}

void run_all_tests_nonblocking()
{
	testsDone = false;
	testsPassed = false;

	// Some platforms require a Client to be created before using pplx::task
	config = Configuration::create(endpoint, accountId, applicationName);
	config->logger = logger;
	client = Client::create(config);

	tests.push_back(test_connect);
	tests.push_back(test_echo);
	tests.push_back(test_rpc_server);
	tests.push_back(test_rpc_server_cancel);
	tests.push_back(test_rpc_server_exception);
	tests.push_back(test_rpc_server_clientException);
	tests.push_back(test_rpc_client);
	tests.push_back(test_rpc_client_cancel);
	tests.push_back(test_rpc_client_exception);
	tests.push_back(test_syncclock);
	tests.push_back(test_disconnect);
	tests.push_back(test_clean);

	execNextTest();
}

bool tests_done()
{
    return testsDone;
}

bool tests_passed()
{
    return testsPassed;
}
