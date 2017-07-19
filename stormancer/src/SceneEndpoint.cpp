#include "stdafx.h"
#include "SceneEndpoint.h"

namespace Stormancer
{
	SceneEndpoint::SceneEndpoint()
	{
	}

	SceneEndpoint::SceneEndpoint(const std::string& token, const ConnectionData& tokenData)
		: token(token)
		, tokenData(tokenData)
	{
	}

	SceneEndpoint::~SceneEndpoint()
	{
	}
};
