#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace Stormancer
{
	/// Connection informations to send to the server.
	class ConnectionData
	{
	public:

		std::string AccountId;
		std::string Application;
		std::string ContentType;
		std::string DeploymentId;
		std::shared_ptr<std::unordered_map<std::string, std::string>> Endpoints;
		int64 Expiration;
		int64 Issued;
		std::string Routing;
		std::string SceneId;
		std::string UserData;
		int32 Version;

		MSGPACK_DEFINE_MAP(AccountId, Application, ContentType, DeploymentId, Endpoints, Expiration, Issued, Routing, SceneId, UserData, Version);
	};
}
