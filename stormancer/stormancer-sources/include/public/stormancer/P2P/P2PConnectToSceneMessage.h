#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/RouteDto.h"
#include "stormancer/msgpack_define.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace Stormancer
{
	struct P2PConnectToSceneMessage
	{
		/// Scene id
		std::string sceneId;

		/// Scene handle
		byte sceneHandle;

		/// List of client routes.
		std::vector<RouteDto> routes;

		/// Scene Metadatas.
		std::unordered_map<std::string, std::string> sceneMetadata;

		/// Connection Metadatas.
		std::unordered_map<std::string, std::string> connectionMetadata;

		MSGPACK_DEFINE(sceneId, sceneHandle, routes, sceneMetadata, connectionMetadata);
	};
}
