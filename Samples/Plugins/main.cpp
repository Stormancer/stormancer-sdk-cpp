#include "StormancerWrapper.h"
#include "PlayerProfile/PlayerProfileService.h"
#include "PlayerProfile/PlayerProfile.h"
#include "Leaderboard/LeaderboardService.h"
#include "stormancer/Logger/ConsoleLogger.h"
#include "stormancer/Helpers.h"

bool stop = false;
const std::string endpoint = "http://127.0.0.1:8081";
const std::string accountId = "account-id";
const std::string applicationName = "application-name";

int main()
{
	Stormancer::StormancerWrapper stormancerWrapper;

	auto logger = std::make_shared<Stormancer::ConsoleLogger>();

	stormancerWrapper.setLogger(logger);

	stormancerWrapper.init(endpoint, accountId, applicationName);
	
	stormancerWrapper.Authenticate("steam", "plop").then([logger, &stormancerWrapper](pplx::task<void> t) {
		try
		{
			t.wait();
			logger->log("connect OK");
			stormancerWrapper.getService<Stormancer::LeaderboardService>("services", true, true).then([=](pplx::task<std::shared_ptr<Stormancer::LeaderboardService>> t) {
				try
				{
					auto service = t.get();
					Stormancer::LeaderboardQuery query;
					query.leaderboardName = "test";
					query.size = 10;
					service->query(query).then([=](pplx::task<Stormancer::LeaderboardResult> t2) {
						try
						{
							auto result = t2.get();
							auto resultsCount = result.results.size();
							logger->log(Stormancer::LogLevel::Debug, "main", "Results count = " + std::to_string(resultsCount));
							if (resultsCount > 0)
							{
								logger->log(Stormancer::LogLevel::Debug, "main", "My rank = " + std::to_string(result.results[0].ranking));
								logger->log(Stormancer::LogLevel::Debug, "main", "My score = " + std::to_string(result.results[0].scoreRecord.score));
								std::time_t a = result.results[0].scoreRecord.createdOn / 1000;
								auto dateStr = Stormancer::time_tToStr(a, true);
								logger->log(Stormancer::LogLevel::Debug, "main", "My date = " + dateStr);
							}
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
			});
		}
		catch (const std::exception& ex)
		{
			logger->log(std::string("connect failed (") + ex.what() + ")");
		}
	});
	
	/*stormancerWrapper.getService<Stormancer::PlayerProfileService<Stormancer::PlayerProfile>>("main").then([](std::shared_ptr<Stormancer::PlayerProfileService<Stormancer::PlayerProfile>> pps) {
		pps->get().then([](Stormancer::PlayerProfile pp) {
			std::cout << "get player profile OK" << std::endl;
		});
	});*/

	// simulate a game render loop
	pplx::create_task([&stormancerWrapper]() {
		while (!stop)
		{
			// We should call update at render loop start
			stormancerWrapper.update();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	});

	std::cin.ignore();

	stormancerWrapper.shutdown();

	stop = true;

	return 0;
}
