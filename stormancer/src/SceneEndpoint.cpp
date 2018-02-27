#include "stormancer/stdafx.h"
#include "stormancer/SceneEndpoint.h"

namespace Stormancer
{
	SceneEndpoint::SceneEndpoint()
	{
	}

	SceneEndpoint::SceneEndpoint(const std::string& token, const ConnectionData& tokenData)
		: tokenData(tokenData)
		, token(token)
	{
	}

	SceneEndpoint::~SceneEndpoint()
	{
	}
};
