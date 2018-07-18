#pragma once
#include "msgpack.hpp"
#include "stormancer\RouteDto.h"
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