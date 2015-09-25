#pragma once
#include "headers.h"
#include "RequestModuleBuilder.h"

namespace Stormancer
{
	/// Handle system requests.
	class IRequestModule
	{
	public:
		virtual void registerModule(RequestModuleBuilder* builder) = 0;
	};
};
