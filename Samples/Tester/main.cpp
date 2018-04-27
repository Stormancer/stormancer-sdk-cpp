#include "test.h"
//#include "testP2P.h"
#include <iostream>

int main(int, char**)
{
	{
		Stormancer::Tester tester;

		tester.run_all_tests();

		//testP2P();
		//p2pClient();

		std::cin.ignore();
	}

	return 0;
}
