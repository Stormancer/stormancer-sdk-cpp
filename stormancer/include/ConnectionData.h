#pragma once

#include "headers.h"

namespace Stormancer
{
	/// Connection informations to send to the server.
	class ConnectionData
	{
	public:

		ConnectionData();
		virtual ~ConnectionData();
		void msgpack_unpack(msgpack::object const& o);

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
	};
};
