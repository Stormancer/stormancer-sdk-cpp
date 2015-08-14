#pragma once
#include "headers.h"

namespace Stormancer
{
	class PluginBuildContext;

	class IClientPlugin
	{
	public:
		IClientPlugin();
		virtual ~IClientPlugin();

		virtual void build(PluginBuildContext* ctx) = 0;
	};
};
