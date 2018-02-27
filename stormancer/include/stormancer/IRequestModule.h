#pragma once

#include "stormancer/headers.h"
#include "stormancer/RequestModuleBuilder.h"

namespace Stormancer
{
	/// Handle system requests.
	class IRequestModule
	{
	public:

		virtual void registerModule(RequestModuleBuilder* builder) = 0;
	};
};
