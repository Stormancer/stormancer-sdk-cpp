#pragma once
#include "headers.h"
#include "RequestModuleBuilder.h"

namespace Stormancer
{
	/// Handle system requests.
	class IRequestModule
	{
	public:
		IRequestModule();
		virtual ~IRequestModule();

		virtual void registerModule(RequestModuleBuilder* builder) = 0;
	};
};
