#pragma once

#include "stormancer/BuildConfig.h"

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
