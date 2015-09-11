#include "stormancer.h"

namespace Stormancer
{
	Configuration::Configuration(std::string account, std::string application)
		: account(account),
		application(application),
		dispatcher(new DefaultPacketDispatcher()),
		scheduler(new DefaultScheduler())
	{
		transportFactory = defaultTransportFactory;

		//plugins.push_back(new RpcClientPlugin());
	}

	Configuration::~Configuration()
	{
	}

	Configuration& Configuration::metadata(std::string key, std::string value)
	{
		_metadata[key] = value;
		return *this;
	}

	std::string Configuration::getApiEndpoint()
	{
		return (serverEndpoint.length() ? serverEndpoint : apiEndpoint);
	}

	/*
	void Configuration::addPlugin(IClientPlugin* plugin)
	{
		plugins.push_back(plugin);
	}
	*/
};
