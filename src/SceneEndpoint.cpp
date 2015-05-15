
#include "SceneEndpoint.h"

namespace Stormancer
{
	SceneEndpoint::SceneEndpoint()
	{
	}

	SceneEndpoint::SceneEndpoint(wstring& token, ConnectionData& tokenData)
		: token(token),
		tokenData(tokenData)
	{
	}

	SceneEndpoint::~SceneEndpoint()
	{
	}
};
