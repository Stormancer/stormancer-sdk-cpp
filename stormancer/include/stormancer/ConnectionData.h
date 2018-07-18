#pragma once

#include "stormancer/headers.h"

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
		std::map<std::string, std::string> Endpoints;
		int64 Expiration;
		int64 Issued;
		std::string Routing;
		std::string SceneId;
		std::string UserData;
		int32 Version;

		MSGPACK_DEFINE_MAP(AccountId, Application, ContentType, DeploymentId, Endpoints, Expiration, Issued, Routing, SceneId, UserData, Version);
	};
};
