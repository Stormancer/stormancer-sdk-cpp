#include "testP2P.h"
#include "test.h"
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
	std::string endpoint = (argc >= 2) ? argv[1] : "http://gc3.stormancer.com:81";
	std::string account = (argc >= 3) ? argv[2] : "test";
	std::string application = (argc >= 4) ? argv[3] : "test-application";

	{
		int nbParallelTests = 1;
		std::vector<pplx::task<void>> tasks;
		std::vector<std::shared_ptr<Stormancer::Tester>> testers;
		for (int i = 0; i < nbParallelTests; i++)
		{
			auto tester = std::make_shared<Stormancer::Tester>(endpoint, account, application, "test-scene");
			testers.push_back(tester);
			tasks.push_back(tester->run_all_tests_nonblocking());
		}

		pplx::when_all(tasks.begin(), tasks.end()).get();
		std::cin.ignore();
	}

	{
		int guestsCount = 1;
		testP2P(endpoint, account, application, "test-scene", guestsCount)
			.then([](pplx::task<void> task)
				{
					try
					{
						task.get();
					}
					catch (const std::exception& ex)
					{
						std::clog << "P2P tests failed: " << ex.what() << std::endl;
					}
				});
		std::cin.ignore();
	}

	

	

	return 0;
}
