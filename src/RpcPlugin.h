#pragma once
#include "headers.h"
#include "IClientPlugin.h"

namespace Stormancer
{
	class RpcPlugin : public IClientPlugin
	{
	public:
		void build(PluginBuildContext* ctx);

	public:
		STORMANCER_DLL_API static const char* pluginName;
		STORMANCER_DLL_API static const char* serviceName;
		STORMANCER_DLL_API static const char* version;
		STORMANCER_DLL_API static const char* nextRouteName;
		STORMANCER_DLL_API static const char* errorRouteName;
		STORMANCER_DLL_API static const char* completeRouteName;
		STORMANCER_DLL_API static const char* cancellationRouteName;
	};
};
