#pragma once
#include "headers.h"
#include "../RakNet/Source/MessageIdentifiers.h"

namespace Stormancer
{
	/// Used by the protocol for identifying the nature of the system messages.
	enum class MessageIDTypes
	{
		/// Connect to a scene.
		ID_CONNECT_TO_SCENE = DefaultMessageIDTypes::ID_USER_PACKET_ENUM,

		/// Disconnect from a scene.
		ID_DISCONNECT_FROM_SCENE = 135,

		/// Retrieve scene informations.
		ID_GET_SCENE_INFOS = 136,

		/// Response to a request.
		ID_REQUEST_RESPONSE_MSG = 137,

		/// Request completed.
		ID_REQUEST_RESPONSE_COMPLETE = 138,

		/// Response failed.
		ID_REQUEST_RESPONSE_ERROR = 139,

		/// Connection informations.
		ID_CONNECTION_RESULT = 140,

		///Scenes
		ID_SCENES = 141,
	};
};
