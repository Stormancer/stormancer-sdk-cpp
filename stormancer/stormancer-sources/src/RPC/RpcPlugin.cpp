#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcPlugin.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/RPC/Constants.h"
namespace Stormancer
{
	

	void RpcPlugin::registerSceneDependencies(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(Stormancer::rpc::pluginName);

			if (rpcParams == Stormancer::rpc::version)
			{
				scene->dependencyResolver()->registerDependency<RpcService>([=](std::weak_ptr<DependencyResolver> resolver) {
					return std::make_shared<RpcService>(scene, resolver.lock()->resolve<IActionDispatcher>());
				}, true);
			}
		}
	}

	void RpcPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(Stormancer::rpc::pluginName);

			if (rpcParams == Stormancer::rpc::version)
			{
				auto rpc = scene->dependencyResolver()->resolve<RpcService>();

				scene->addRoute(Stormancer::rpc::nextRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver()->resolve<RpcService>().get();
					rpcService->next(p);
				});

				scene->addRoute(Stormancer::rpc::cancellationRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver()->resolve<RpcService>().get();
					rpcService->cancel(p);
				});

				scene->addRoute(Stormancer::rpc::errorRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver()->resolve<RpcService>().get();
					rpcService->error(p);
				});

				scene->addRoute(Stormancer::rpc::completeRouteName, [scene](Packetisp_ptr p)
				{
					auto rpcService = scene->dependencyResolver()->resolve<RpcService>().get();
					rpcService->complete(p);
				});
			}
		}
	}

	void RpcPlugin::sceneDisconnected(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto dr = scene->dependencyResolver();
			if (dr)
			{
				auto rpcService = dr->resolve<RpcService>();
				rpcService->cancelAll("Scene disconnected");
			}		
		}
	}
};
