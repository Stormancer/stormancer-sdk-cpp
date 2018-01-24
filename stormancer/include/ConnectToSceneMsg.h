#pragma once

#include "headers.h"
#include "RouteDto.h"

namespace Stormancer
{
	/// Required parameters to connect to a scene.
	class ConnectToSceneMsg
	{
	public:

		/// Authentication token.
		std::string Token;

		/// List of client routes.
		std::vector<RouteDto> Routes;

		/// Scene Metadatas.
		std::map<std::string, std::string> SceneMetadata;

		/// Connection Metadatas.
		std::map<std::string, std::string> ConnectionMetadata;

		MSGPACK_DEFINE_MAP(Token, Routes, SceneMetadata, ConnectionMetadata);
	};
};
