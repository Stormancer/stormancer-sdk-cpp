#include "stormancer.h"

namespace Stormancer
{
	RouteDto::RouteDto()
	{
	}

	RouteDto::RouteDto(bytestream* stream)
	{
		deserialize(stream);
	}

	RouteDto::~RouteDto()
	{
	}

	void RouteDto::serialize(bytestream* stream)
	{
		// TODO
	}

	void RouteDto::deserialize(bytestream* stream)
	{
		// TODO
	}
}
