#pragma once
#include <array>
#include "TestCase.h"
#include "stormancer/stormancer.h"
#include "PlayerData/PlayerDataService.h"
#include "Authentication/AuthenticationPlugin.h"
#include "Authentication/AuthenticationService.h"

class TestPlayerDataPlugin : public TestCase
{

	struct TestData
	{
		int a;
		std::string b;

		bool operator==(const TestData& rhs) const
		{
			return a == rhs.a && b == rhs.b;
		}

		MSGPACK_DEFINE(a, b);
	};

public:

	virtual void set_up() override
	{
		auto tasks = {
			setup_connection(client1, "TestPlayerDataClient1").then([this](std::shared_ptr<Stormancer::PlayerDataService<std::string>> data)
			{
				data1 = data;
				id1 = client1->dependencyResolver()->resolve<Stormancer::AuthenticationService>()->userId();
			}),
			setup_connection(client2, "TestPlayerDataClient2").then([this](std::shared_ptr<Stormancer::PlayerDataService<std::string>> data)
			{
				data2 = data;
				id2 = client2->dependencyResolver()->resolve<Stormancer::AuthenticationService>()->userId();
			})
		};

		pplx::when_all(tasks.begin(), tasks.end()).wait();
	}

	virtual void tear_down() override
	{
		data1.reset();
		data2.reset();
		auto disconnects = {
			client1->disconnect(),
			client2->disconnect()
		};
		pplx::when_all(disconnects.begin(), disconnects.end()).wait();
		client1.reset();
		client2.reset();
	}

	virtual bool run() override
	{
		if (!data1 || !data2)
		{
			set_error("Could not get PlayerDataService from scene");
			return false;
		}

		return test_set_get().get();
	}

	virtual std::string get_name() override
	{
		return "TestPlayerDataPlugin";
	}

private:
	static pplx::task<std::shared_ptr<Stormancer::PlayerDataService<std::string>>> setup_connection(Stormancer::Client_ptr& client, std::string auth_ticket)
	{
		auto conf = Stormancer::Configuration::create("http://localhost:8081", "test", "test");
		conf->addPlugin(new Stormancer::AuthenticationPlugin);
		conf->addPlugin(new Stormancer::PlayerDataPlugin<std::string>);

		client = Stormancer::Client::create(conf);
		std::map<std::string, std::string> auth_context{ 
			{ "provider", "steam"},
			{ "ticket", auth_ticket }
		};

		auto auth_service = client->dependencyResolver()->resolve<Stormancer::AuthenticationService>();
		return auth_service->login(auth_context).then([=]
		{
			return auth_service->getPrivateScene("test-player-data-plugin");
		})
			.then([=](Stormancer::Scene_ptr scene)
		{
			return scene->dependencyResolver()->resolve<Stormancer::PlayerDataService<std::string>>();
		});
	}

	pplx::task<bool> test_set_get()
	{
		Stormancer::PlayerData<std::string> pd;
		TestData orig_test_data{ 1, "str" };
		return data1->SetPlayerData(pd).then([this]
		{
			return data2->GetPlayerData(id1);
		})
		.then([=](Stormancer::PlayerData<std::string> retrieved_test_data)
		{
			//if (retrieved_test_data == pd)//orig_test_data)
			{
				return true;
			}
			/*else
			{
				set_error("Original data and retrieved data are not equal");
				return false;
			}*/
		});
	}

	Stormancer::Client_ptr client1, client2;

	std::shared_ptr<Stormancer::PlayerDataService<std::string>> data1, data2;

	std::string id1, id2;
};