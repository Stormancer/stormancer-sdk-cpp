#include "stormancer/stdafx.h"
#include "stormancer/Configuration.h"
#include "stormancer/RakNet/RakNetTransport.h"
#include "stormancer/RPC/RpcPlugin.h"

namespace Stormancer
{
	Configuration::Configuration(const std::string& endpoint, const std::string& account, const std::string& application)
		: account(account)
		, application(application)			
	{
		scheduler = std::make_shared<DefaultScheduler>();
		transportFactory = _defaultTransportFactory;
		dispatcher = [](DependencyResolver* dr) {
			return std::make_shared<DefaultPacketDispatcher>(dr->resolve<ILogger>());
		};
		addServerEndpoint(endpoint);
		_plugins.push_back(new RpcPlugin());
	}

	Configuration::~Configuration()
	{
	}

	std::shared_ptr<Configuration> Configuration::create(const std::string& endpoint, const std::string& account, const std::string& application)
	{
		if (account.empty() || application.empty() || endpoint.empty())
		{
			throw std::invalid_argument("Check your account and application parameters");
		}

		return std::shared_ptr<Configuration>(new Configuration(endpoint, account, application));
	}

	Configuration& Configuration::setMetadata(const std::string& key, const std::string& value)
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

	void Configuration::addServerEndpoint(const std::string& serverEndpoint)
	{
		_serverEndpoints.push_back(std::string(serverEndpoint));
	}

	std::vector<std::string> Configuration::getApiEndpoint()
	{
		return _serverEndpoints;
	}

	const std::function<std::shared_ptr<ITransport>(DependencyResolver*)> Configuration::_defaultTransportFactory = [](DependencyResolver* resolver)
	{
		return std::make_shared<RakNetTransport>(resolver);
	};

	const bool Configuration::hasPublicIp()
	{
		return dedicatedServerEndpoint != "";
	}

	const std::string Configuration::getIp_Port()
	{
		return dedicatedServerEndpoint + ":" + std::to_string(serverGamePort); 
	}
};
