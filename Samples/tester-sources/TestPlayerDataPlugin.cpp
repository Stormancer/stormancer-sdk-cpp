#include "TestPlayerDataPlugin.h"

void TestPlayerDataPlugin::set_up(TestParameters params)
{
	auto tasks = {
		setup_connection(client1, "TestPlayerDataClient1", params).then([this](std::shared_ptr<Stormancer::PlayerDataService> data)
	{
		data1 = data;
		id1 = client1->dependencyResolver().lock()->resolve<Stormancer::AuthenticationService>()->userId();
	}),
		setup_connection(client2, "TestPlayerDataClient2", params).then([this](std::shared_ptr<Stormancer::PlayerDataService> data)
	{
		data2 = data;
		id2 = client2->dependencyResolver().lock()->resolve<Stormancer::AuthenticationService>()->userId();
	})
	};

	pplx::when_all(tasks.begin(), tasks.end()).wait();
}

void TestPlayerDataPlugin::tear_down()
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

bool TestPlayerDataPlugin::run()
{
	try
	{
		if (!data1 || !data2)
		{
			set_error("Could not get PlayerDataService from scene");
			return false;
		}

		std::string datakey("key");
		std::string data("value");

		std::vector<pplx::task<void>> tasks;
		tasks.push_back(check_data_updated(data1, datakey, data, id1));
		tasks.push_back(check_data_updated(data2, datakey, data, id1));

		data1->SetPlayerData(datakey, data);
		pplx::when_all(tasks.begin(), tasks.end()).get();
	}
	catch (std::exception& ex)
	{
		set_error(ex.what());
		return false;
	}
	return true;
}

pplx::task<std::shared_ptr<Stormancer::PlayerDataService>> TestPlayerDataPlugin::setup_connection(Stormancer::Client_ptr& client, std::string auth_ticket, TestParameters params)
{
	auto conf = Stormancer::Configuration::create(params.endpoint, params.account, params.application);
	conf->addPlugin(new Stormancer::AuthenticationPlugin);
	conf->addPlugin(new Stormancer::PlayerDataPlugin);

	client = Stormancer::Client::create(conf);


	auto auth_service = client->dependencyResolver().lock()->resolve<Stormancer::AuthenticationService>();
	auth_service->getCredentialsCallback = [auth_ticket]() {

		std::unordered_map<std::string, std::string> auth_context{
			{ "provider", "steam" },
		{ "ticket", auth_ticket }
		};
		return pplx::task_from_result(auth_context);
	};
	return auth_service->login().then([=]
	{
		return auth_service->connectToPrivateScene("test-player-data-plugin");
	})
		.then([](Stormancer::Scene_ptr scene)
	{
		return scene->dependencyResolver().lock()->resolve<Stormancer::PlayerDataService>();
	});
}

pplx::task<void> TestPlayerDataPlugin::check_data_updated(std::shared_ptr<Stormancer::PlayerDataService> service, std::string expected_datakey, std::string expected_data, std::string expected_user)
{
	pplx::task_completion_event<void> tce;

	service->OnPlayerDataUpdated([expected_data, expected_user, expected_datakey, tce](const Stormancer::PlayerDataUpdate& update)
	{
		if (update.Data.Data == expected_data &&
			update.DataKey == expected_datakey &&
			update.PlayerId == expected_user)
		{
			tce.set();
		}
		else
		{
			tce.set_exception("unexpected data update");
		}
	});

	return pplx::create_task(tce);
}
