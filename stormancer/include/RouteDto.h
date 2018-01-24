#pragma once

#include "headers.h"

namespace Stormancer
{
	/// Information about a scene route.
	struct RouteDto
	{
	public:

		std::string Name;

		uint16 Handle;

		std::map<std::string, std::string> Metadata;

		MSGPACK_DEFINE_MAP(Name, Handle, Metadata);
	};
};
