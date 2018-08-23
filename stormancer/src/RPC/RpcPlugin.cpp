#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcPlugin.h"
#include "stormancer/RPC/RpcService.h"
#include "stormancer/IActionDispatcher.h"

namespace Stormancer
{
	const std::string RpcPlugin::pluginName = "stormancer.plugins.rpc";
	const std::string RpcPlugin::serviceName = "rpcService";
	const std::string RpcPlugin::version = "1.1.0";
	const std::string RpcPlugin::nextRouteName = "stormancer.rpc.next";
	const std::string RpcPlugin::errorRouteName = "stormancer.rpc.error";
	const std::string RpcPlugin::completeRouteName = "stormancer.rpc.completed";
	const std::string RpcPlugin::cancellationRouteName = "stormancer.rpc.cancel";

	void RpcPlugin::registerSceneDependencies(Scene* scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				scene->dependencyResolver().lock()->registerDependency<RpcService>([=](std::weak_ptr<DependencyResolver> resolver) {
					return std::make_shared<RpcService>(scene->shared_from_this(), resolver.lock()->resolve<IActionDispatcher>());
				}, true);
			}
		}
	}

	void RpcPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				auto rpc = scene->dependencyResolver().lock()->resolve<RpcService>();

				scene->addRoute(nextRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>().get();
					rpcService->next(p);
				});

				scene->addRoute(cancellationRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>().get();
					rpcService->cancel(p);
				});

				scene->addRoute(errorRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>().get();
					rpcService->error(p);
				});

				scene->addRoute(completeRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>().get();
					rpcService->complete(p);
				});
			}
		}
	}

	void RpcPlugin::sceneDisconnected(Scene* scene)
	{
		if (scene)
		{
			auto dr = scene->dependencyResolver().lock();
			if (dr)
			{
				auto rpcService = dr->resolve<RpcService>();
				rpcService->cancelAll("Scene disconnected");
			}		
		}
	}
};
