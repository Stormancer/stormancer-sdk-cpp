#pragma once

#include "headers.h"

namespace Stormancer
{
	/// State of a network connection.
	enum ConnectionState
	{
		/// disconnected.
		Disconnected = 0,
		
		/// Connecting.
		Connecting = 1,
		
		/// Connected.
		Connected = 2,

		Disconnecting = 3
	};
};
