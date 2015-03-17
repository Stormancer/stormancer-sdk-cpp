#include "Configuration/ClientConfiguration.h"

namespace Stormancer
{
	ClientConfiguration ClientConfiguration::forAccount(std::string account, std::string application)
	{
		return ClientConfiguration(account, application, false);
	}

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
};
