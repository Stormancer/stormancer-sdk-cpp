#pragma once

#include "stormancer/headers.h"

namespace Stormancer
{
	class ServerDescriptor
	{
	public:
		std::string id;
		std::string hostName;
		uint16 port;
	};
}
