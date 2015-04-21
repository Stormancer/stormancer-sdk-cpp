#include "stormancer.h"

namespace Stormancer
{
	EmptyDto::EmptyDto()
	{
	}

	EmptyDto::EmptyDto(bytestream* stream)
		: ISerializable(stream)
	{
	}

	EmptyDto::~EmptyDto()
	{
	}

	void EmptyDto::serialize(bytestream* stream)
	{
	}

	void EmptyDto::deserialize(bytestream* stream)
	{
	}
}
