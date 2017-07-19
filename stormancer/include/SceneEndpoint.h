#pragma once

#include "headers.h"
#include "ConnectionData.h"

namespace Stormancer
{
	/// Informations to connect to a scene.
	class SceneEndpoint
	{
	public:
		SceneEndpoint();
		SceneEndpoint(const std::string& token, const ConnectionData& tokenData);
		virtual ~SceneEndpoint();

	public:
		ConnectionData tokenData;
		std::string token;
	};
};
