#pragma once
#include "stdafx.h"
#include "Plugins/PluginBuildContext.h"

namespace Stormancer
{
	class IClientPlugin
	{
	public:
		IClientPlugin();
		virtual ~IClientPlugin();

		virtual void build(PluginBuildContext ctx) = 0;
	};
};
