#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	struct EmptyDto : public ISerializable
	{
	public:
		EmptyDto();
		EmptyDto(bytestream* stream);
		virtual ~EmptyDto();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);
	};
};
