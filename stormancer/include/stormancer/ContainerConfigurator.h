#pragma once

#include "stormancer/headers.h"
#include "stormancer/DependencyResolver.h"
#include "stormancer/Configuration.h"

namespace Stormancer
{
	void ConfigureContainer(DependencyResolver* dr, Configuration_ptr config);
};
