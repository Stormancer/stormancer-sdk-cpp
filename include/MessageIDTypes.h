#pragma once
#include "headers.h"
#include "../RakNet/Source/MessageIdentifiers.h"

namespace Stormancer
{
	enum class MessageIDTypes
	{
		ID_CONNECT_TO_SCENE = DefaultMessageIDTypes::ID_USER_PACKET_ENUM,

		ID_DISCONNECT_FROM_SCENE = 135,

		ID_GET_SCENE_INFOS = 136,

		ID_REQUEST_RESPONSE_MSG = 137,

		ID_REQUEST_RESPONSE_COMPLETE = 138,

		ID_REQUEST_RESPONSE_ERROR = 139,

		ID_CONNECTION_RESULT = 140,

		ID_SCENES = 141,
	};
};
