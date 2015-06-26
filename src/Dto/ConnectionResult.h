#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Result of a connection attempt.
	class ConnectionResult
	{
	public:
	
		/// Constructor.
		ConnectionResult();

		/// Destructor.
		virtual ~ConnectionResult();

		/// MessagePack deserialization.
		void msgpack_unpack(msgpack::object const& o);

	public:
		/// Handle of the scene the client was connected to.
		byte SceneHandle;
		
		/// Route mappings in the scene (ie : routeName => routeHandle)
		std::map<std::string, uint16> RouteMappings;
	};
};
