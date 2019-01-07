#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/DependencyResolver.h"
#include "stormancer/Configuration.h"

namespace Stormancer
{
	void ConfigureContainer(std::weak_ptr<DependencyResolver> dr, Configuration_ptr config);
};
