#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/RouteDto.h"

namespace Stormancer
{
	/// Required parameters to connect to a scene.
	class ConnectToSceneMessage
	{
	public:

		/// Authentication token.
		std::string Token;

		/// List of client routes.
		std::vector<RouteDto> Routes;

		/// Scene Metadatas.
		std::unordered_map<std::string, std::string> SceneMetadata;

		/// Connection Metadatas.
		std::unordered_map<std::string, std::string> ConnectionMetadata;

		MSGPACK_DEFINE_MAP(Token, Routes, SceneMetadata, ConnectionMetadata);
	};
}
