#pragma once
#include "headers.h"
#include "IClientPlugin.h"

class RpcClientPlugin : public Stormancer::IClientPlugin
{
public:
	void build(Stormancer::PluginBuildContext& ctx);

public:
	static const char* pluginName;
	static const char* serviceName;
	static const char* version;
	static const char* nextRouteName;
	static const char* errorRouteName;
	static const char* completeRouteName;
	static const char* cancellationRouteName;
};
