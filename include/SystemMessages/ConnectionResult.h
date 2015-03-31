#pragma once
#include "headers.h"

namespace Stormancer
{
	class ConnectionResult
	{
	public:
		ConnectionResult();
		ConnectionResult(byte sceneHandle, map<wstring, uint16> routeMappings)
			: SceneHandle(sceneHandle),
			RouteMappings(routeMappings)
		{
		}
		virtual ~ConnectionResult();

	public:
		byte SceneHandle;
		map<wstring, uint16> RouteMappings;
	};
};
