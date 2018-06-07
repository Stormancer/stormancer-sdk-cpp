#include "testP2P.h"
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
	std::string endpoint = (argc >= 2) ? argv[1] : "http://api.stormancer.com:8081/";
	std::string account = (argc >= 3) ? argv[2] : "samples";
	std::string application = (argc >= 4) ? argv[3] : "p2p";

	setTestParameters(endpoint, account, application);

	testP2P();
	//p2pClient();
	std::cin.ignore();

	return 0;
}