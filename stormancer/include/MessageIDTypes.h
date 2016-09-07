#pragma once
#include "headers.h"
#include "MessageIdentifiers.h"

namespace Stormancer
{
	/// Used by the protocol for identifying the nature of the system messages
	enum class MessageIDTypes
	{
		/// System request
		ID_SYSTEM_REQUEST = DefaultMessageIDTypes::ID_USER_PACKET_ENUM,

		/// Reponse to a system request
		ID_REQUEST_RESPONSE_MSG = 137,

		/// "Request complete" message to close a system request channel
		ID_REQUEST_RESPONSE_COMPLETE = 138,

		/// Rrror as a response to a system request and close the request channel
		ID_REQUEST_RESPONSE_ERROR = 139,

		/// Identifies a response to a connect to scene message
		ID_CONNECTION_RESULT = 140,

		/// First id for scene handles
		ID_SCENES = 141
	};
};
