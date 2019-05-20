#include "stormancer/stdafx.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	void Serializer::serialize(obytestream&) const
	{
		// do nothing
	}

	/*void Serializer::UnstackAndDeserialize(const byte*, const uint64, uint64*) const
	{
		// do nothing
	}*/

	template<>
	void Serializer::deserializeOne(ibytestream&) const
	{
		// do nothing
	}

	template<>
	void Serializer::deserializeOne(const byte*, const uint64, uint64*) const
	{
		// do nothing
	}
};
