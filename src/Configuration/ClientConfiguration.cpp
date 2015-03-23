#include "libs.h"
#include "Configuration/ClientConfiguration.h"
#include "Infrastructure/MsgPackSerializer.h"
#include "Infrastructure/DefaultPacketDispatcher.h"
#include "Transports/RaknetTransport.h"
#include "DefaultLogger.h"
#include "Plugins/RpcClientPlugin.h"

namespace Stormancer
{
	ClientConfiguration::ClientConfiguration(string account, string application)
		: account(account),
		application(application)/*,
		dispatcher(make_shared<IPacketDispatcher*>(new DefaultPacketDispatcher())),
		transport(RaknetTransport(DefaultLogger::instance())),
		maxPeers(20)*/
	{
		/*serializers.push_back(MsgPackSerializer());
		plugins.push_back(make_shared<IClientPlugin*>(new RpcClientPlugin()));*/
	}

	ClientConfiguration::~ClientConfiguration()
	{
	}

	/*string ClientConfiguration::getApiEndpoint()
	{
		return (serverEndpoint.length() ? serverEndpoint : apiEndpoint);
	}

	ClientConfiguration& ClientConfiguration::metadata(string key, string value)
	{
		_metadata[key] = value;
		return *this;
	}

	void ClientConfiguration::addPlugin(shared_ptr<IClientPlugin*> plugin)
	{
		plugins.push_back(plugin);
	}*/
};
