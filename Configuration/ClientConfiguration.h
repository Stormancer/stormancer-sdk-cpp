#pragma once
#include <string>

namespace Stormancer
{
	class ClientConfiguration
	{
	private:

	public:
		std::string account;
		std::string application;
		std::string serverEndpoint;
		bool isLocalDev;

		ClientConfiguration();
		ClientConfiguration(std::string account, std::string application, bool isLocalDev = false);
		~ClientConfiguration();

		static ClientConfiguration forAccount(std::string account, std::string application);
	};
};
