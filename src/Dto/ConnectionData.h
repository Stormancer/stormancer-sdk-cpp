#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Connection informations to send to the server.
	class ConnectionData
	{
	public:

		/// Constructor.
		ConnectionData();

		/// Destructor.
		virtual ~ConnectionData();

		/// MessagePack deserialization.
		void msgpack_unpack(msgpack::object const& o);

	public:
		std::string AccountId;
		std::string Application;
		std::string ContentType;
		std::string DeploymentId;
		stringMap Endpoints;
		time_t Expiration;
		time_t Issued;
		std::string Routing;
		std::string SceneId;
		std::string UserData;
	};
};
