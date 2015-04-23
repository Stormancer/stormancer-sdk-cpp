#include "stormancer.h"

namespace Stormancer
{
	ConnectionResult::ConnectionResult()
	{
	}

	ConnectionResult::ConnectionResult(bytestream* stream)
	{
		deserialize(stream);
	}

	ConnectionResult::~ConnectionResult()
	{
	}

	void ConnectionResult::serialize(bytestream* stream)
	{
		// TODO
	}

	void ConnectionResult::deserialize(bytestream* stream)
	{
		// TODO
	}
}
