#include "stormancer.h"

namespace Stormancer
{
	const char* RpcPlugin::pluginName = "stormancer.plugins.rpc";
	const char* RpcPlugin::serviceName = "rpcService";
	const std::string RpcPlugin::version = "1.1.0";
	const char* RpcPlugin::nextRouteName = "stormancer.rpc.next";
	const char* RpcPlugin::errorRouteName = "stormancer.rpc.error";
	const char* RpcPlugin::completeRouteName = "stormancer.rpc.completed";
	const char* RpcPlugin::cancellationRouteName = "stormancer.rpc.cancel";

	void RpcPlugin::registerSceneDependencies(Scene* scene)
	{
		scene->dependencyResolver()->registerDependency<IRpcService>([scene](DependencyResolver* resolver) {

			return std::make_shared<RpcService>(scene, resolver->resolve<IActionDispatcher>());
			/*return std::shared_ptr<RpcService>(new RpcService(scene), [](RpcService* service) {
				printf("test");
			});*/
		}, true);
	}

	void RpcPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				auto rpc = scene->dependencyResolver()->resolve<IRpcService>();

				scene->addRoute(nextRouteName, [scene](Packetisp_ptr p) {
					auto rpc = scene->dependencyResolver()->resolve<IRpcService>().get();

					static_cast<RpcService*>(rpc)->next(p);
				});
				scene->addRoute(cancellationRouteName, [scene](Packetisp_ptr p) {
					auto rpc = scene->dependencyResolver()->resolve<IRpcService>().get();

					static_cast<RpcService*>(rpc)->cancel(p);
				});
				scene->addRoute(errorRouteName, [scene](Packetisp_ptr p) {
					auto rpc = scene->dependencyResolver()->resolve<IRpcService>().get();

					static_cast<RpcService*>(rpc)->error(p);
				});
				scene->addRoute(completeRouteName, [scene](Packetisp_ptr p) {
					auto rpc = scene->dependencyResolver()->resolve<IRpcService>().get();

					static_cast<RpcService*>(rpc)->complete(p);
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
		delete this;
	}
};
