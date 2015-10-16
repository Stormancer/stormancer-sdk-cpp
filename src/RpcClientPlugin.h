#pragma once
#include "headers.h"
#include "IClientPlugin.h"

namespace Stormancer
{
	class RpcClientPlugin : public IClientPlugin
	{
	public:
		void build(PluginBuildContext& ctx);

	public:
		static const std::string pluginName;
		static const std::string serviceName;
		static const std::string version;
		STORMANCER_DLL_API static const std::string nextRouteName;
		STORMANCER_DLL_API static const std::string errorRouteName;
		STORMANCER_DLL_API static const std::string completeRouteName;
		static const std::string cancellationRouteName;
	};
};
