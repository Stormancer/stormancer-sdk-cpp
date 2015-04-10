#pragma once
#include "headers.h"

namespace Stormancer
{
	struct ConnectionResult
	{
		byte SceneHandle;
		map<wstring, uint16> RouteMappings;
	};
};
