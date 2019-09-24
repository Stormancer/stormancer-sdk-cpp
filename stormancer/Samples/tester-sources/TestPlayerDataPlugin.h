#pragma once
#include <array>
#include "TestCase.h"
#include "PlayerData/PlayerDataService.h"
#include "Authentication/AuthenticationPlugin.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/IClient.h"

class TestPlayerDataPlugin : public TestCase
{

public:

	virtual void set_up(TestParameters) override;

	virtual void tear_down() override;

	virtual bool run() override;

	virtual std::string get_name() override
	{
		return "TestPlayerDataPlugin";
	}

private:
	static pplx::task<std::shared_ptr<Stormancer::PlayerDataService>> setup_connection(Stormancer::Client_ptr& client, std::string auth_ticket, TestParameters);

	pplx::task<void> check_data_updated(std::shared_ptr<Stormancer::PlayerDataService> service, std::string expected_datakey, std::string expected_data, std::string expected_user);

	Stormancer::Client_ptr client1, client2;

	std::shared_ptr<Stormancer::PlayerDataService> data1, data2;

	std::string id1, id2;
};