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
				auto processor = new RpcService(scene);
				scene->dependencyResolver()->registerDependency<IRpcService>(processor);
				scene->addRoute(nextRouteName, [processor](Packetisp_ptr p) {
					processor->next(p);
				});
				scene->addRoute(cancellationRouteName, [processor](Packetisp_ptr p) {
					processor->cancel(p);
				});
				scene->addRoute(errorRouteName, [processor](Packetisp_ptr p) {
					processor->error(p);
				});
				scene->addRoute(completeRouteName, [processor](Packetisp_ptr p) {
					processor->complete(p);
				});
			}
		}
	}

	void RpcPlugin::sceneDisconnected(Scene* scene)
	{
		if (scene)
		{
			auto processor = scene->dependencyResolver()->resolve<IRpcService>();
			processor->disconnected();
		}
	}

	void RpcPlugin::destroy()
	{
		delete this;
	}
};
