#include "stormancer.h"

namespace Stormancer
{
	Configuration::Configuration(wstring account, wstring application)
		: account(account),
		application(application),
		dispatcher(new DefaultPacketDispatcher),
		transport(new RakNetTransport(ILogger::instance()))
	{
		//plugins.push_back(new RpcClientPlugin());
	}

	Configuration::~Configuration()
	{
	}

	wstring Configuration::getApiEndpoint()
	{
		return (serverEndpoint.length() ? serverEndpoint : apiEndpoint);
	}

	Configuration& Configuration::setMetadata(wstring key, wstring value)
	{
		metadata[key] = value;
		return *this;
	}

	/*void Configuration::addPlugin(IClientPlugin* plugin)
	{
		plugins.push_back(plugin);
	}*/
};
