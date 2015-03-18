#include "Configuration/ClientConfiguration.h"
#include "Infrastructure/MsgPackSerializer.h"
#include "Infrastructure/DefaultPacketDispatcher.h"
#include "Transports/RaknetTransport.h"
#include "DefaultLogger.h"

namespace Stormancer
{
	ClientConfiguration ClientConfiguration::forAccount(string account, string application)
	{
		return ClientConfiguration(account, application);
	}

	ClientConfiguration::ClientConfiguration(string account, string application)
		: account(account),
		application(application),
		isLocalDev(false),
		dispatcher(DefaultPacketDispatcher()),
		logger(DefaultLogger::instance()),
		transport(RaknetTransport(logger)),
		maxPeers(20)
	{
		serializers.push_back(MsgPackSerializer());
		plugins.push_back(RpcClientPlugin());
	}

	ClientConfiguration::~ClientConfiguration()
	{
	}
};
