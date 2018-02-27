#pragma once

#include "headers.h"

namespace Stormancer
{
	struct TransformMetadata
	{
		const std::map<std::string, std::string> sceneMetadata;
		std::string sceneId;
		std::string routeName;
		int sceneHandle;
		int routeHandle;
	};
}
