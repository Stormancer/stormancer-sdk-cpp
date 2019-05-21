#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/DependencyInjection.h"
#include "stormancer/Configuration.h"

namespace Stormancer
{
	void ConfigureContainer(ContainerBuilder& builder, Configuration_ptr config);
};
