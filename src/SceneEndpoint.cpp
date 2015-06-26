#include "stormancer.h"

namespace Stormancer
{
	SceneEndpoint::SceneEndpoint()
	{
	}

	SceneEndpoint::SceneEndpoint(std::string& token, ConnectionData& tokenData)
		: token(token),
		tokenData(tokenData)
	{
	}

	SceneEndpoint::~SceneEndpoint()
	{
	}
};
