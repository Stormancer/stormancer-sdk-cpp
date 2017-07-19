#include "stdafx.h"
#include "EmptyDto.h"

namespace Stormancer
{
	EmptyDto::EmptyDto()
	{
	}

	EmptyDto::~EmptyDto()
	{
	}

	void EmptyDto::msgpack_unpack(msgpack::object const&)
	{
		// do nothing
	}
}
