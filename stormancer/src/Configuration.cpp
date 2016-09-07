#include "stormancer.h"

namespace Stormancer
{
	Configuration::Configuration(const std::string endpoint, const std::string account, const std::string application)
		: account(account),
		application(application),
		dispatcher(new DefaultPacketDispatcher()),
		scheduler(new DefaultScheduler())
	{
		transportFactory = defaultTransportFactory;
		addServerEndpoint(endpoint);
		_plugins.push_back(new RpcPlugin());
	}

	Configuration::~Configuration()
	{
	}

	std::shared_ptr<Configuration> Configuration::create(const std::string endpoint, const std::string account, const std::string application)
	{
		if (account == "" || application == "" || endpoint == "")
		{
			throw std::invalid_argument("Check your account and application parameters");
		}

		return std::shared_ptr<Configuration>(new Configuration(endpoint, account, application));
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

	void Configuration::addServerEndpoint(const std::string serverEndpoint)
	{
		_serverEndpoints.push_back(std::string(serverEndpoint));
	}

	std::vector<std::string> Configuration::getApiEndpoint()
	{
		return _serverEndpoints;
	}
};
