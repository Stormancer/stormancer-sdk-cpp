#include "stormancer.h"

namespace Stormancer
{
	ClientConfiguration::ClientConfiguration(wstring account, wstring application)
		: account(account),
		application(application),
		//dispatcher(make_shared<IPacketDispatcher*>(new DefaultPacketDispatcher())),
		//transport(RaknetTransport(DefaultLogger::instance())),
		maxPeers(20)
	{
		//serializers.push_back(MsgPackSerializer());
		//plugins.push_back(make_shared<IClientPlugin*>(new RpcClientPlugin()));
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

	/*void ClientConfiguration::addPlugin(shared_ptr<IClientPlugin*> plugin)
	{
		plugins.push_back(plugin);
	}*/
};
