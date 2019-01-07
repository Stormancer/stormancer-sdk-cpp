#include "stormancer/stdafx.h"
#include "stormancer/ConnectionState.h"

namespace Stormancer
{
	ConnectionState::ConnectionState(State state2)
		: state(state2)
	{
	}

	ConnectionState::ConnectionState(State state2, std::string reason2)
		: state(state2)
		, reason(reason2)
	{
	}

	ConnectionState& ConnectionState::operator=(State state2)
	{
		state = state2;
		return *this;
	}

	bool ConnectionState::operator==(ConnectionState& other) const
	{
		return state == other.state;
	}

	bool ConnectionState::operator!=(ConnectionState& other) const
	{
		return state != other.state;
	}

	bool ConnectionState::operator==(State state2) const
	{
		return state == state2;
	}

	bool ConnectionState::operator!=(State state2) const
	{
		return state != state2;
	}

	ConnectionState::operator int()
	{
		return (int)state;
	}
};
