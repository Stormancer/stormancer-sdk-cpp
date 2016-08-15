#include "stormancer.h"

namespace Stormancer
{
	const char* RpcPlugin::pluginName = "stormancer.plugins.rpc";
	const char* RpcPlugin::serviceName = "rpcService";
	const char* RpcPlugin::version = "1.1.0";
	const char* RpcPlugin::nextRouteName = "stormancer.rpc.next";
	const char* RpcPlugin::errorRouteName = "stormancer.rpc.error";
	const char* RpcPlugin::completeRouteName = "stormancer.rpc.completed";
	const char* RpcPlugin::cancellationRouteName = "stormancer.rpc.cancel";

	void RpcPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (strcmp(rpcParams, version) == 0)
			{
				auto rpcService = new RpcService(scene);
				scene->dependencyResolver()->registerDependency<IRpcService>(rpcService);
				scene->addRoute(nextRouteName, [rpcService](Packetisp_ptr p) {
					rpcService->next(p);
				});
				scene->addRoute(cancellationRouteName, [rpcService](Packetisp_ptr p) {
					rpcService->cancel(p);
				});
				scene->addRoute(errorRouteName, [rpcService](Packetisp_ptr p) {
					rpcService->error(p);
				});
				scene->addRoute(completeRouteName, [rpcService](Packetisp_ptr p) {
					rpcService->complete(p);
				});
			}
		}
	}

	void RpcPlugin::sceneDisconnected(Scene* scene)
	{
		if (scene)
		{
			auto rpcService = scene->dependencyResolver()->resolve<IRpcService>();
			rpcService->cancelAll("Scene disconnected");
		}
	}

	void RpcPlugin::destroy()
	{
		Stormancer::destroy(this);
	}
};
