#pragma once
#include "headers.h"
#include "Plugins/IClientPlugin.h"

namespace Stormancer
{
	class RpcClientPlugin : public IClientPlugin
	{
	public:
		RpcClientPlugin();
		virtual ~RpcClientPlugin();

		void build(PluginBuildContext ctx);

	public:
		const string nextRouteName = "stormancer.rpc.next";
		const string errorRouteName = "stormancer.rpc.error";
		const string completedRouteName = "stormancer.rpc.completed";
		const string version = "1.0.0";
		const string pluginName = "stormancer.plugins.rpc";

	private:

	};
};
