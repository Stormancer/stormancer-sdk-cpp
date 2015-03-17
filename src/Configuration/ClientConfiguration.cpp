#include "stdafx.h"
#include "ClientConfiguration.h"

namespace Stormancer
{
	ClientConfiguration::ClientConfiguration()
		: isLocalDev(false)
	{
	}

	ClientConfiguration::ClientConfiguration(std::string account, std::string application, bool isLocalDev)
		: account(account),
		application(application),
		isLocalDev(isLocalDev)
	{
	}

	ClientConfiguration::~ClientConfiguration()
	{
	}

	ClientConfiguration ClientConfiguration::forAccount(std::string account, std::string application)
	{
		ClientConfiguration config = ClientConfiguration(account, application);
		config.isLocalDev = false;
		return config;
	}
};
