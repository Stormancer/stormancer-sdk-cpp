#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcPlugin.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/RPC/Constants.h"
#include "stormancer/Helpers.h"
#include "stormancer/Utilities/PointerUtilities.h"

namespace Stormancer
{
	

	void RpcPlugin::registerSceneDependencies(ContainerBuilder& sceneBuilder, std::shared_ptr<Scene> scene)
	{
		auto rpcParams = scene->getHostMetadata(Stormancer::rpc::pluginName);

		if (rpcParams == Stormancer::rpc::version)
		{
			sceneBuilder.registerDependency<RpcService, Scene, IActionDispatcher>().singleInstance();
		}
	}

	void RpcPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto rpcParams = scene->getHostMetadata(Stormancer::rpc::pluginName);

			if (rpcParams == Stormancer::rpc::version)
			{
				std::weak_ptr<Scene> sceneWeak(scene);

				scene->addRoute(Stormancer::rpc::nextRouteName, [sceneWeak](Packetisp_ptr p)
				{
					auto scene = sceneWeak.lock();
					if (scene)
					{
						auto rpcService = scene->dependencyResolver().resolve<RpcService>();
						auto logger = scene->dependencyResolver().resolve<ILogger>();
						logger->log(LogLevel::Trace, "RpcService", "Received rpc.next message on scene", scene->id());
						rpcService->next(p);
					}
				});

				scene->addRoute(Stormancer::rpc::cancellationRouteName, [sceneWeak](Packetisp_ptr p)
				{
					auto scene = sceneWeak.lock();
					if (scene)
					{
						auto rpcService = scene->dependencyResolver().resolve<RpcService>();
						auto logger = scene->dependencyResolver().resolve<ILogger>();
						logger->log(LogLevel::Trace, "Received rpc.cancel message on scene", scene->id());
						rpcService->cancel(p);
					}
				});

				scene->addRoute(Stormancer::rpc::errorRouteName, [sceneWeak](Packetisp_ptr p)
				{
					auto scene = sceneWeak.lock();
					if (scene)
					{
						auto rpcService = scene->dependencyResolver().resolve<RpcService>();
						auto logger = scene->dependencyResolver().resolve<ILogger>();
						logger->log(LogLevel::Trace, "RpcService", "Received rpc.error message on scene", scene->id());
						rpcService->error(p);
					}
				});

				scene->addRoute(Stormancer::rpc::completeRouteName, [sceneWeak](Packetisp_ptr p)
				{
					auto scene = sceneWeak.lock();
					if (scene)
					{
						auto rpcService = scene->dependencyResolver().resolve<RpcService>();
						auto logger = scene->dependencyResolver().resolve<ILogger>();
						logger->log(LogLevel::Trace, "RpcService", "Received rpc.complete message on scene", scene->id());
						rpcService->complete(p);
					}
				});
			}
		}
	}

	void RpcPlugin::sceneDisconnected(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto& dr = scene->dependencyResolver();
			auto rpcService = dr.resolve<RpcService>();
			rpcService->cancelAll("Scene disconnected");
		}
	}
}
