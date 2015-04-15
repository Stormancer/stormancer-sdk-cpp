#pragma once
#include "headers.h"
#include "RequestModuleBuilder.h"

namespace Stormancer
{
	class IRequestModule
	{
	public:
		IRequestModule();
		virtual ~IRequestModule();

		virtual void registerModule(RequestModuleBuilder* builder) = 0;
	};
};
