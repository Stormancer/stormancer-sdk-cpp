#pragma once
#include "headers.h"
#include "IPlugin.h"

namespace Stormancer
{
	class RpcPlugin : public IPlugin
	{
	public:
		void sceneCreated(Scene* scene);
		
		void sceneDisconnected(Scene* scene);

		void destroy();

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
