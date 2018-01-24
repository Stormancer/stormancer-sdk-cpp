#pragma once

#include "headers.h"

namespace Stormancer
{
	/// Information about a scene route.
	struct DisconnectFromSceneDto
	{
	public:

		byte SceneHandle;

		MSGPACK_DEFINE_MAP(SceneHandle);
	};
};
