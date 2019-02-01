#pragma once
#include "stormancer/RPC/service.h"
#include "stormancer/IClient.h"
#include "stormancer/Logger/ConsoleLogger.h"




namespace Stormancer
{
	class Tester
	{
	public:

#pragma region public_methods

		Tester(std::string endpoint, std::string account, std::string application, std::string sceneId)
			: _endpoint(endpoint)
			, _accountId(account)
			, _applicationName(application)
			, _sceneName(sceneId)
		{
		}

		Tester() = default;

		/// Run the test suite, and block indefinitely on stdin.
		void run_all_tests();

		/// Same as run_all_tests(), but returns immediately.
		/// Use tests_done() and tests_passed() to check the tests' status.
		void run_all_tests_nonblocking();

		/// Check whether the tests are done (NOT if they're succesful).
		bool tests_done();

		/// Check whether or not the tests were succsful.
		/// You need to check that the tests have finished running with tests_done() before calling this function.
		bool tests_passed();

#pragma endregion

	private:

#pragma region private_methods

		void execNextTest();
		std::string connectionStateToString(ConnectionState connectionState);
		void onEcho(Packetisp_ptr packet);
		void onMessage(Packetisp_ptr packet);
		pplx::task<void> test_rpc_client_received(RpcRequestContext_ptr rc);
		pplx::task<void> test_rpc_client_cancel_received(RpcRequestContext_ptr rc);
		pplx::task<void> test_rpc_client_exception_received(RpcRequestContext_ptr rc);
		
		void test_create();
		void test_connect();
		void test_echo();
		void test_rpc_server();
		void test_rpc_server_cancel();
		void test_rpc_server_exception();
		void test_rpc_server_clientException();
		void test_rpc_client();
		void test_rpc_client_cancel();
		void test_rpc_client_exception();
		void test_syncClock();
		void test_setServerTimeout();
		void test_disconnect();
		void test_disconnectWithReason();
		void test_clean();
		void test_Ping_Cluster();

#pragma endregion

#pragma region private_members




		ILogger_ptr _logger = std::make_shared<ConsoleLogger>();

		bool _stop = false;
		const std::string _endpoint = "https://api2.stormancer.com";
		const std::string _accountId = "tester";
		const std::string _applicationName = "test-application";
		const std::string _sceneName = "main";
		bool _disconnectWithReasonRequested = false;
		const std::string _disconnectReason = "DisconnectParticularReason";
		const std::string _echoMessage = "hello";
		Configuration_ptr _config;
		std::shared_ptr<IClient> _client;
		std::weak_ptr<Scene> _sceneMain;

		std::deque<std::function<void()>> _tests;
		bool _testsDone = false;
		bool _testsPassed = false;

		pplx::task_completion_event<void> testCompletedTce;

#pragma endregion
	};
}
