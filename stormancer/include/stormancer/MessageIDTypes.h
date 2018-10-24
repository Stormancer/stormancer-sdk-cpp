#pragma once

#include "stormancer/headers.h"


namespace Stormancer
{
	/// Used by the protocol for identifying the nature of the system messages
	enum class MessageIDTypes
	{
		/// Re-use RakNet NAT messages id types
		ID_P2P_RELAY = 58,
		ID_P2P_TUNNEL = 59,
		ID_ADVERTISE_PEERID = 48, // Borrow id from autopatcher

		ID_CLOSE_REASON = 115,

		/// System request
		ID_SYSTEM_REQUEST = 134, // = DefaultMessageIDTypes::ID_USER_PACKET_ENUM

		ID_ENCRYPTED = 135,

		/// Reponse to a system request
		ID_REQUEST_RESPONSE_MSG = 137,

		/// "Request complete" message to close a system request channel
		ID_REQUEST_RESPONSE_COMPLETE = 138,

		/// Rrror as a response to a system request and close the request channel
		ID_REQUEST_RESPONSE_ERROR = 139,

		/// Identifies a response to a connect to scene message
		ID_CONNECTION_RESULT = 140,

		/// First id for scene handles
		ID_SCENES = 141,
	};
};
