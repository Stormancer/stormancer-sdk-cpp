#pragma once
#include "libs.h"
#include "Infrastructure/Modules/RequestModuleBuilder.h"

namespace Stormancer
{
	class IRequestModule
	{
	public:
		IRequestModule();
		virtual ~IRequestModule();

		virtual void registerModule(RequestModuleBuilder builder) = 0;
	};
};
