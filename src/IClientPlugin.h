#pragma once
#include "headers.h"
#include "PluginBuildContext.h"

namespace Stormancer
{
	class IClientPlugin
	{
	public:
		virtual void build(PluginBuildContext ctx) = 0;
	};
};
