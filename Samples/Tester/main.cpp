#include "test.h"
#include "testP2P.h"
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
	std::string endpoint = (argc >= 2) ? argv[1] : "https://api2.stormancer.com";
	std::string account = (argc >= 3) ? argv[2] : "tester";
	std::string application = (argc >= 4) ? argv[3] : "tester";

	/*{
		Stormancer::Tester tester(endpoint, account, application);

		tester.run_all_tests();

		
	}*/
	
	testP2P(endpoint, account, application, "test-scene");
	std::cin.ignore();
	return 0;
}
