#pragma once

#include "stormancer/headers.h"
#include "stormancer/SceneEndpoint.h"

namespace Stormancer
{
	/// Interface for a token handler
	class ITokenHandler
	{
	public:

		//Protocol v1
		virtual SceneEndpoint decodeToken(const std::string& token) = 0;

		//Protocol v2
		virtual SceneEndpoint getSceneEndpointInfo(const std::string& token) = 0;
	};
};
