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
		_plugins.push_back(new RpcPlugin());
	}

	Configuration::~Configuration()
	{
	}

	Configuration* Configuration::forAccount(const char* account, const char* application)
	{
		if (!account || !application)
		{
			throw std::invalid_argument("Check your account and application parameters");
		}

		return new Configuration(account, application);
	}

	Configuration& Configuration::metadata(const char* key, const char* value)
	{
		_metadata[key] = value;
		return *this;
	}

	void Configuration::addPlugin(IPlugin* plugin)
	{
		_plugins.push_back(plugin);
	}

	const std::vector<IPlugin*> Configuration::plugins()
	{
		return _plugins;
	}

	void Configuration::addServerEndpoint(const char* serverEndpoint)
	{
		_serverEndpoints.push_back(std::string(serverEndpoint));
	}

	std::vector<std::string> Configuration::getApiEndpoint()
	{
		if (_serverEndpoints.size())
		{
			return _serverEndpoints;
		}
		else
		{
			return std::vector<std::string>{ apiEndpoint };
		}
	}
};
