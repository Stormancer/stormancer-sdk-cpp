#pragma once
#include "headers.h"
#include "IClientPlugin.h"

namespace Stormancer
{
	class RpcClientPlugin : public IClientPlugin
	{
	public:
		void build(PluginBuildContext ctx);

	public:
		static const std::string pluginName = "stormancer.plugins.rpc";
		static const std::string serviceName = "rpcService";
		static const std::string version = "1.1.0";
		static const std::string nextRouteName = "stormancer.rpc.next";
		static const std::string errorRouteName = "stormancer.rpc.error";
		static const std::string completeRouteName = "stormancer.rpc.completed";
		static const std::string cancellationRouteName = "stormancer.rpc.cancel";
	};
};
