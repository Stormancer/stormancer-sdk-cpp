#pragma once
#include "headers.h"
#include "Plugins/IClientPlugin.h"
#include "Plugins/PluginBuildContext.h"

namespace Stormancer
{
	class RpcClientPlugin : public IClientPlugin
	{
	public:
		RpcClientPlugin();
		virtual ~RpcClientPlugin();

		void build(PluginBuildContext* ctx);

	public:
		const std::string nextRouteName = "stormancer.rpc.next";
		const std::string errorRouteName = "stormancer.rpc.error";
		const std::string completedRouteName = "stormancer.rpc.completed";
		const std::string version = "1.0.0";
		const std::string pluginName = "stormancer.plugins.rpc";

	private:

	};
};
