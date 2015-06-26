#include "stormancer.h"

namespace Stormancer
{
	EmptyDto::EmptyDto()
	{
	}

	EmptyDto::~EmptyDto()
	{
	}

	void EmptyDto::msgpack_unpack(msgpack::object const& o)
	{
		// do nothing
	}
}
