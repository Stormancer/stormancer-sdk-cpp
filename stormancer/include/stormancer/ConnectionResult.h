#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include <map>
#include <string>

namespace Stormancer
{
	/// Result of a connection attempt.
	class ConnectionResult
	{
	public:

		/// Handle of the scene the client was connected to.
		byte SceneHandle;
		
		/// Route mappings in the scene (ie : routeName => routeHandle)
		std::map<std::string, uint16> RouteMappings;

		MSGPACK_DEFINE_MAP(SceneHandle, RouteMappings);
	};
};
