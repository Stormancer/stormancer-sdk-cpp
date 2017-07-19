#pragma once

#include "headers.h"
#include "DependencyResolver.h"
#include "Configuration.h"

namespace Stormancer
{
	void ConfigureContainer(DependencyResolver* dr, Configuration_ptr config);
};
