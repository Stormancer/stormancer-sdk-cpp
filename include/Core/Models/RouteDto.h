#pragma once
#include "stdafx.h"

namespace Stormancer
{
	struct RouteDto
	{
		std::string Name;
		uint16_t Handle;
		std::map<std::string, std::string> Metadata;
	};
};
