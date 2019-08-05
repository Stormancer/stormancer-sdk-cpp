#pragma once

#include "stormancer/IClient.h"
#include "stormancer/Configuration.h"
#include "stormancer/IPlugin.h"
#include "Users/Users.hpp"
#include "test.h"
#include "IAuthenticationTester.h"
#include <memory>

class TestUsersPlugin
{
private:
	class TestAuthProvider1 : public Stormancer::Users::IAuthenticationEventHandler
	{
		// Inherited via IAuthenticationEventHandler
		virtual pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
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
		virtual pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
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
		TestPlatformId(std::string id) : PlatformUserId("test", id) {}
	};

	class TestCurrentUserAuthProvider : public Stormancer::Users::IAuthenticationEventHandler
	{
		virtual pplx::task<void> retrieveCredentials(const Stormancer::Users::CredientialsContext & context) override
		{
			if (context.platformUserId == nullptr || context.platformUserId->platform != "test")
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

public:
	static void runTests(const Stormancer::Tester& tester)
	{
		testEventHandlers(tester);
		testEventHandlersWithCallback(tester);
		testNoEventHandlers(tester);
		testNoEventHandlersWithCallback(tester);
		testCurrentLocalUser(tester);

		auto authTester = IAuthenticationTester::create();
		authTester->runTests(tester);
	}

private:

	static void testEventHandlers(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlers");

		auto conf = Configuration::create(tester._endpoint, tester._accountId, tester._applicationName);
		conf->logger = tester._logger;
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

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlers PASSED");
	}

	static void testNoEventHandlers(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlers");

		auto conf = Configuration::create(tester._endpoint, tester._accountId, tester._applicationName);
		conf->logger = tester._logger;
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
			tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlers PASSED");
			return;
		}
		throw std::runtime_error("login() should have failed because of no credentials");
	}

	static void testNoEventHandlersWithCallback(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlersWithCallback");

		auto conf = Configuration::create(tester._endpoint, tester._accountId, tester._applicationName);
		conf->logger = tester._logger;
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

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testNoEventHandlersWithCallback PASSED");
	}

	static void testEventHandlersWithCallback(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlersWithCallback");

		auto conf = Configuration::create(tester._endpoint, tester._accountId, tester._applicationName);
		conf->logger = tester._logger;
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

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testEventHandlersWithCallback PASSED");
	}

	static void testCurrentLocalUser(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testCurrentLocalUser");

		auto conf = Configuration::create(tester._endpoint, tester._accountId, tester._applicationName);
		conf->logger = tester._logger;
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

		tester._logger->log(LogLevel::Info, "TestUsersPlugin", "testCurrentLocalUser PASSED");
	}

	static void assertex(bool condition, std::string message)
	{
		if (!condition)
		{
			throw std::runtime_error(message.c_str());
		}
	}

};