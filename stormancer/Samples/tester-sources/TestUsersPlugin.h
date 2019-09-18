#pragma once

#include "stormancer/IClient.h"
#include "stormancer/Configuration.h"
#include "stormancer/IPlugin.h"
#include "Users/Users.hpp"
#include "test.h"
#include "TestHelpers.h"
#include "IAuthenticationTester.h"
#include <memory>

class TestUsersPlugin
{
private:
	class TestAuthProvider1 : public Stormancer::Users::IAuthenticationEventHandler
	{
		// Inherited via IAuthenticationEventHandler
		pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
		{
			context.authParameters->type = "test";
			passed = true;
			return pplx::task_from_result();
		}
	public:
		bool passed = false;
	};
	class TestAuthProvider2 : public Stormancer::Users::IAuthenticationEventHandler
	{
		// Inherited via IAuthenticationEventHandler
		pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
		{
			context.authParameters->parameters["testkey"] = "testvalue";
			passed = true;
			return pplx::task_from_result();
		}

	public:
		bool passed = false;
	};

	class TestPlugin : public Stormancer::IPlugin
	{
		void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
		{
			builder.registerDependency(prvd1).as<Stormancer::Users::IAuthenticationEventHandler>();
			builder.registerDependency(prvd2).as<Stormancer::Users::IAuthenticationEventHandler>();
		}
	public:
		std::shared_ptr<TestAuthProvider1> prvd1 = std::make_shared<TestAuthProvider1>();
		std::shared_ptr<TestAuthProvider2> prvd2 = std::make_shared<TestAuthProvider2>();
	};

	struct TestPlatformId : public Stormancer::Users::PlatformUserId
	{
		std::string type() const override { return "test"; }
		TestPlatformId(std::string id) : PlatformUserId(id) {}
	};

	class TestCurrentUserAuthProvider : public Stormancer::Users::IAuthenticationEventHandler
	{
		virtual pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
		{
			if (context.platformUserId == nullptr || context.platformUserId->type() != "test")
			{
				throw std::runtime_error("invalid platform user id");
			}
			context.authParameters->type = "test";
			context.authParameters->parameters["testkey"] = "testvalue";
			return pplx::task_from_result();
		}
	};

	class TestPlatformIdPlugin : public Stormancer::IPlugin
	{
		void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
		{
			builder.registerDependency<TestCurrentUserAuthProvider>().as<Stormancer::Users::IAuthenticationEventHandler>();
		}
	};

	class TestCredsRenewalHandler : public Stormancer::Users::IAuthenticationEventHandler
	{
	public:
		pplx::task_completion_event<void> tce;
		// Wait for the initial session data to be received before updating it
		pplx::task<void> initialDataReceived;

		TestCredsRenewalHandler(pplx::task_completion_event<void> tce, pplx::task<void> initialDataReceived) : tce(tce), initialDataReceived(initialDataReceived) {}

		pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
		{
			// renew credentials in 5 seconds
			context.authParameters->parameters["testrenewal"] = "00:00:05";
			return pplx::task_from_result();
		}

		pplx::task<void> renewCredentials(const Stormancer::Users::CredentialsRenewalContext& context) override
		{
			context.response->parameters["testData"] = "updatedData";
			tce.set();
			return initialDataReceived;
		}
	};

	class TestCredsRenewalPlugin : public Stormancer::IPlugin
	{
		void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
		{
			builder.registerDependency<TestCredsRenewalHandler>([this](const Stormancer::DependencyScope&)
			{
				return std::make_shared<TestCredsRenewalHandler>(renewCredentialsCalledTce, pplx::create_task(initialDataReceivedTce));
			}).as<Stormancer::Users::IAuthenticationEventHandler>();
		}

		pplx::task_completion_event<void> renewCredentialsCalledTce;
		pplx::task_completion_event<void> initialDataReceivedTce;

	public:
		pplx::task<void> renewCredentialsCalled() {
			return pplx::create_task(renewCredentialsCalledTce);
		}

		void setInitialDataReceived() { initialDataReceivedTce.set(); }
	};

public:
	static void runTests(const Stormancer::Tester& tester)
	{
		testEventHandlers(tester);
		testEventHandlersWithCallback(tester);
		testNoEventHandlers(tester);
		testNoEventHandlersWithCallback(tester);
		testCurrentLocalUser(tester);
		testSendRequest(tester);
		testRenewCredentials(tester);

		auto authTester = IAuthenticationTester::create();
		authTester->runTests(tester);
	}

private:

	static void testEventHandlers(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlers");

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		//conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);
		TestPlugin* plugin = new TestPlugin;
		conf->addPlugin(plugin);

		auto client = IClient::create(conf);

		auto users = client->dependencyResolver().resolve<Users::UsersApi>();

		assertex(plugin->prvd1->passed == false, "event handler should not have been called yet");
		assertex(plugin->prvd2->passed == false, "event handler should not have been called yet");

		users->login().get();

		assertex(plugin->prvd1->passed == true, "event handler 1 should have been called");
		assertex(plugin->prvd2->passed == true, "event handler 2 should have been called");

		client->disconnect().wait();

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlers PASSED");
	}

	static void testNoEventHandlers(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlers");

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		//conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);

		auto client = IClient::create(conf);

		auto users = client->dependencyResolver().resolve<Users::UsersApi>();

		try
		{
			users->login().get();
		}
		catch (const std::exception&)
		{
			// OK
			client->disconnect().wait();
			tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlers PASSED");
			return;
		}
		throw std::runtime_error("login() should have failed because of no credentials");
	}

	static void testNoEventHandlersWithCallback(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlersWithCallback");

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		//conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);

		auto client = IClient::create(conf);

		auto users = client->dependencyResolver().resolve<Users::UsersApi>();
		bool passed = false;
		users->getCredentialsCallback = [&passed]
		{
			Users::AuthParameters params;
			params.type = "test";
			params.parameters["testkey"] = "testvalue";
			passed = true;
			return pplx::task_from_result<Users::AuthParameters>(params);
		};

		users->login().get();

		assertex(passed == true, "the credentials callback should have been called");
		client->disconnect().wait();

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlersWithCallback PASSED");
	}

	static void testEventHandlersWithCallback(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlersWithCallback");

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		//conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);
		TestPlugin* plugin = new TestPlugin;
		conf->addPlugin(plugin);

		auto client = IClient::create(conf);

		auto users = client->dependencyResolver().resolve<Users::UsersApi>();
		bool passed = false;
		users->getCredentialsCallback = [&passed]
		{
			passed = true;
			return pplx::task_from_result<Users::AuthParameters>(Users::AuthParameters());
		};

		users->login().get();

		assertex(passed == true, "the credentials callback should have been called");
		assertex(plugin->prvd1->passed == true, "event handler 1 should have been called");
		assertex(plugin->prvd2->passed == true, "event handler 2 should have been called");
		client->disconnect().wait();

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlersWithCallback PASSED");
	}

	static void testCurrentLocalUser(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testCurrentLocalUser");

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		//conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);
		conf->addPlugin(new TestPlatformIdPlugin);

		auto client = IClient::create(conf);

		auto users = client->dependencyResolver().resolve<Users::UsersApi>();

		assertex(users->getCurrentLocalUser() == nullptr, "initial local user should be null");

		try
		{
			users->login().get();
			throw std::runtime_error("login() should have failed because TestCurrentUserAuthProvider should have thrown");
		}
		catch (const Users::CredentialsException&)
		{
			// expected
		}

		auto user = std::make_shared<TestPlatformId>("testId");
		users->setCurrentLocalUser(user).get();
		users->login().get();
		assertex(users->getCurrentLocalUser() == user, "local user should be 'testId'");

		auto user2 = std::make_shared<TestPlatformId>("testId2");
		users->setCurrentLocalUser(user2).get();
		assertex(users->getCurrentLocalUser() == user2, "local user should be 'testId2'");
		assertex(users->connectionState() == Users::GameConnectionState::Authenticated, "new user should be authenticated after the local user change");

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testCurrentLocalUser PASSED");
	}

	static std::shared_ptr<Stormancer::IClient> makeTestClient(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		//conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);
		conf->addPlugin(new TestPlugin);

		return IClient::create(conf);
	}

	static void testSendRequest(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testSendRequest");

		auto sender = makeTestClient(tester);
		auto recipient = makeTestClient(tester);

		auto usersSender = sender->dependencyResolver().resolve<Users::UsersApi>();
		auto usersRcpt = recipient->dependencyResolver().resolve<Users::UsersApi>();
		usersSender->login().get();
		usersRcpt->login().get();

		auto senderId = usersSender->userId();
		std::string requestData = "testData";

		pplx::task_completion_event<void> recipientHandledOp;
		usersRcpt->setOperationHandler("testOp", [recipientHandledOp, senderId, requestData](Users::OperationCtx ctx)
		{
			assertextce(ctx.operation == "testOp", "ctx.operation name should be testOp, but is " + ctx.operation, recipientHandledOp);
			assertextce(ctx.originId == senderId, "ctx.originId should be the sender's Id, but is " + ctx.originId, recipientHandledOp);
			
			try
			{
				Serializer s;
				auto data = s.deserializeOne<std::string>(ctx.request->inputStream());
				assertextce(data == requestData, "data received in the request should be the same as the data that was sent, but it is " + data, recipientHandledOp);
			}
			catch (const std::exception& e)
			{
				recipientHandledOp.set_exception("recipient could not deserialize data, error: " + std::string(e.what()));
			}

			recipientHandledOp.set();
			return pplx::task_from_result();
		});

		SelfObservingTask<void> sendTask = usersSender->sendRequestToUser<void>(usersRcpt->userId(), "testOp", pplx::cancellation_token::none(), requestData);
		
		auto status = cancel_after_timeout(pplx::create_task(recipientHandledOp), 5000).wait();
		assertex(status == pplx::completed, "reception of the request timed out");
		
		status = cancel_after_timeout(*sendTask, 5000).wait();
		assertex(status == pplx::completed, "completion of the request on the sender's side timed out");

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testSendRequest PASSED");
	}

	static void testRenewCredentials(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testRenewCredentials");

		const char* testDataKey = "testData";

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		conf->addPlugin(new Users::UsersPlugin);
		conf->addPlugin(new TestPlugin);
		auto* credsPlugin = new TestCredsRenewalPlugin;
		conf->addPlugin(credsPlugin);
		//conf->logger = tester.logger();

		auto client = IClient::create(conf);
		auto users = client->dependencyResolver().resolve<Users::UsersApi>();
		users->login().get();

		auto scene = users->connectToPrivateScene("test-services-scene").get();
		auto rpc = scene->dependencyResolver().resolve<RpcService>();
		auto sessionData = rpc->rpc<std::string>("users.test.GetSessionData", testDataKey).get();
		assertex(sessionData == "initial", "Initially, session data's 'testData' entry should have the value 'initial' ; instead, it is '" + sessionData + "'");
		credsPlugin->setInitialDataReceived();

		taskDelay(std::chrono::seconds(5)).get();
		assertex(credsPlugin->renewCredentialsCalled().is_done(), "renewCredentials should have been called");

		sessionData = rpc->rpc<std::string>("users.test.GetSessionData", testDataKey).get();
		assertex(sessionData == "updatedData", "session data should now be 'updatedData' ; instead, it is '" + sessionData + "'");

		tester.logger()->log(LogLevel::Info, "TestUsersPlugin", "testRenewCredentials PASSED");
	}

	static void assertex(bool condition, std::string message)
	{
		if (!condition)
		{
			throw std::runtime_error(message.c_str());
		}
	}

	template<typename T>
	static void assertextce(bool condition, std::string message, pplx::task_completion_event<T> tce)
	{
		if (!condition)
		{
			tce.set_exception(std::runtime_error(message.c_str()));
		}
	}
};