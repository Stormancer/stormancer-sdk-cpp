#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/RouteDto.h"
#include "stormancer/msgpack_define.h"
#include <map>
#include <string>
#include <vector>

namespace Stormancer
{
	struct P2PConnectToSceneMessage
	{
		std::string sceneId;
		char sceneHandle;
		std::map<std::string, std::string> metadata;
		std::vector<RouteDto> routes;
		MSGPACK_DEFINE(sceneId, sceneHandle,metadata,routes)
	};
}