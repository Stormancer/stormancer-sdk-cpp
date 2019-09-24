#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/RequestModuleBuilder.h"

namespace Stormancer
{
	/// Handle system requests.
	class IRequestModule
	{
	public:

		virtual ~IRequestModule() = default;

		virtual void registerModule(RequestModuleBuilder& builder) = 0;
	};
}
