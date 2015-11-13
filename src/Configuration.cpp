#include "stormancer.h"

namespace Stormancer
{
	Configuration::Configuration(const char* account, const char* application)
		: account(account),
		application(application),
		dispatcher(new DefaultPacketDispatcher()),
		scheduler(new DefaultScheduler())
	{
		transportFactory = defaultTransportFactory;
	}

	Configuration::~Configuration()
	{
	}

	Configuration_ptr Configuration::forAccount(const char* account, const char* application)
	{
		if (!account || !application)
		{
			throw std::invalid_argument("Check your account and application parameters");
		}

		return Configuration_ptr(new Configuration(account, application));
	}

	Configuration& Configuration::metadata(const char* key, const char* value)
	{
		_metadata[key] = value;
		return *this;
	}

	std::string Configuration::getApiEndpoint()
	{
		return (strlen(serverEndpoint) ? serverEndpoint : apiEndpoint);
	}

	const std::vector<IClientPlugin*>& Configuration::plugins()
	{
		return _plugins;
	}

	void Configuration::addPlugin(IClientPlugin* plugin)
	{
		_plugins.push_back(plugin);
	}
};
