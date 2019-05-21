#include "stormancer/stdafx.h"
#include "stormancer/Configuration.h"
#include "stormancer/RakNet/RakNetTransport.h"
#include "stormancer/RPC/RpcPlugin.h"
#include "stormancer/Logger/NullLogger.h"
#include "stormancer/DefaultScheduler.h"
#include "stormancer/DefaultPacketDispatcher.h"
#include "stormancer/IActionDispatcher.h"

namespace Stormancer
{
	Configuration::Configuration(const std::string& endpoint, const std::string& account, const std::string& application)
		: account(account)
		, application(application)
		, actionDispatcher(std::make_shared<SameThreadActionDispatcher>())
		, logger(std::make_shared<NullLogger>())
		, transportFactory(_defaultTransportFactory)
		, scheduler(std::make_shared<DefaultScheduler>())
	{
		dispatcher = [](const DependencyScope& dr)
		{
			return std::make_shared<DefaultPacketDispatcher>(dr.resolve<ILogger>());
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

		return std::shared_ptr<Configuration>(new Configuration(endpoint, account, application), [](Configuration* ptr) { delete ptr; });
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

	const std::function<std::shared_ptr<ITransport>(const DependencyScope&)> Configuration::_defaultTransportFactory = [](const DependencyScope& scope)
	{
		return std::make_shared<RakNetTransport>(scope);
	};

	const bool Configuration::hasPublicIp()
	{
		return dedicatedServerEndpoint != "";
	}

	const std::string Configuration::getIp_Port()
	{
		return dedicatedServerEndpoint + ":" + std::to_string(serverGamePort);
	}
}
