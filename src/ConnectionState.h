#pragma once
#include "headers.h"

namespace Stormancer
{
	/// State of a network connection.
	enum class ConnectionState
	{
		/// disconnected.
		disconnected,
		
		/// Connecting.
		connecting,
		
		/// Connected.
		connected
	};
};
