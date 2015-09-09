#pragma once
#include "headers.h"

namespace Stormancer
{
	enum class SystemRequestIDTypes
	{
		ID_SET_METADATA = 0,
		ID_SCENE_READY = 1,
		ID_PING = 2,
		// ...
		ID_CONNECT_TO_SCENE = 134,
		ID_DISCONNECT_FROM_SCENE = 135,
		ID_GET_SCENE_INFOS = 136
	};
};
