#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/StormancerTypes.h"
#include "stormancer/RouteDto.h"
#include "stormancer/msgpack_define.h"
#include <string>
#include <map>

namespace Stormancer
{
	/// Informations about a remote scene.
	struct SceneInfosDto
	{
	public:
		
		/// The scene id.
		std::string SceneId;
		
		/// The scene metadatas.
		std::map<std::string, std::string> Metadata;
		
		/// Vector of routes declared on the scene host.
		std::vector<RouteDto> Routes;
		
		/// The serializer the client should use when communicating with the scene.
		std::string SelectedSerializer;

		MSGPACK_DEFINE_MAP(SceneId, Metadata, Routes, SelectedSerializer);
	};
};
