#pragma once

#include "stormancer/BuildConfig.h"
#include <string>

namespace Stormancer
{
	struct ConnectionState
	{
		/// State of a network connection.
		enum State
		{
			/// Disconnected.
			Disconnected = 0,

			/// Connecting.
			Connecting = 1,

			/// Connected.
			Connected = 2,

			/// Disconnecting.
			Disconnecting = 3
		};

		// Methods

		ConnectionState() = default;
		ConnectionState(State state2);
		ConnectionState(State state2, std::string reason2);

		ConnectionState& operator=(State state2);

		bool operator==(ConnectionState& other) const;
		bool operator!=(ConnectionState& other) const;

		bool operator==(State state2) const;
		bool operator!=(State state2) const;

		operator int();

		// Members

		State state = ConnectionState::Disconnected;
		std::string reason;
	};
}
