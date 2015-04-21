#include "stormancer.h"

namespace Stormancer
{
	ClientConfiguration::ClientConfiguration(wstring account, wstring application)
		: account(account),
		application(application),
		dispatcher(new DefaultPacketDispatcher),
		transport(new RakNetTransport(DefaultLogger::instance())),
		maxPeers(20),
		apiEndpoint(L"http://localhost:8081/")
	{
		//plugins.push_back(new RpcClientPlugin());
	}

	ClientConfiguration::~ClientConfiguration()
	{
	}

	wstring ClientConfiguration::getApiEndpoint()
	{
		return (serverEndpoint.length() ? serverEndpoint : apiEndpoint);
	}

	ClientConfiguration& ClientConfiguration::setMetadata(wstring key, wstring value)
	{
		metadata[key] = value;
		return *this;
	}

	/*void ClientConfiguration::addPlugin(IClientPlugin* plugin)
	{
		plugins.push_back(plugin);
	}*/
};
