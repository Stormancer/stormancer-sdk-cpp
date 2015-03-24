#pragma once
#include "headers.h"

namespace Stormancer
{
	struct RouteDto
	{
		string Name;
		uint16 Handle;
		map<string, string> Metadata;
	};
};
