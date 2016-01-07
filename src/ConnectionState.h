#pragma once
#include "headers.h"

namespace Stormancer
{
	/// State of a network connection.
	enum class ConnectionState
	{
		/// disconnected.
		Disconnected,
		
		/// Connecting.
		Connecting,
		
		/// Connected.
		Connected,

		Disconnecting
	};
};
