#pragma once

#include "stormancer/headers.h"
#include "stormancer/SceneEndpoint.h"

namespace Stormancer
{
	/// Interface for a token handler
	class ITokenHandler
	{
	public:

		virtual SceneEndpoint decodeToken(const std::string& token) = 0;
	};
};
