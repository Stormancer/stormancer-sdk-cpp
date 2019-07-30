#include "testP2P.h"
#include "test.h"
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
	std::string endpoint = (argc >= 2) ? argv[1] : "http://gc3.stormancer.com";
	std::string account = (argc >= 3) ? argv[2] : "test";
	std::string application = (argc >= 4) ? argv[3] : "test-application";

	{
		Stormancer::Tester tester(endpoint, account, application, "test-scene");
		tester.run_all_tests();
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
