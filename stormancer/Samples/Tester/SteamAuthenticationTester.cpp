#include "IAuthenticationTester.h"
#include "test.h"
#include "Steam/Steam.hpp"
#include "Users/Users.hpp"
#include <exception>
#include <vector>
#include "steam/steam_api.h"
#include "stormancer/IScheduler.h"

class SteamAuthenticationTester : public IAuthenticationTester
{
public:

	void runTests(const Stormancer::Tester& tester) override
	{
		using namespace Stormancer;

		_logger = tester.logger();

		_logger->log(LogLevel::Info, "SteamAuthenticationTester", "Running Tests");
		try
		{
			auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
			conf->logger = tester.logger();
			conf->addPlugin(new Users::UsersPlugin);
			conf->addPlugin(new Steam::SteamPlugin);
			conf->additionalParameters["steam.initAndRunSteam"] = "true";
			conf->additionalParameters["steam.forcedTicket"] = "MyTicket";

			auto client = IClient::create(conf);
			auto users = client->dependencyResolver().resolve<Users::UsersApi>();
			users->login().get();

			_logger->log(LogLevel::Info, "SteamAuthenticationTester", "User logged in");
			_logger->log(LogLevel::Info, "SteamAuthenticationTester", "Tests PASSED");
		}
		catch (const std::exception& ex)
		{
			tester.logger()->log(LogLevel::Info, "SteamAuthenticationTester", "Tests FAILED", ex);
			std::throw_with_nested(std::runtime_error("SteamAuthenticationTester FAILED"));
		}
	}

private:

	Stormancer::ILogger_ptr _logger;
};

std::unique_ptr<IAuthenticationTester> IAuthenticationTester::create()
{
	return std::unique_ptr<SteamAuthenticationTester>(new SteamAuthenticationTester);
}
