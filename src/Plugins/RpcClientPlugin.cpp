#include "stormancer.h"

namespace Stormancer
{
	RpcClientPlugin::RpcClientPlugin()
	{
	}

	RpcClientPlugin::~RpcClientPlugin()
	{
	}

	void RpcClientPlugin::build(PluginBuildContext* ctx)
	{
		/*
		ctx->sceneCreated += [&](Scene* scene) {
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				auto processor = RpcService(scene);
				scene->registerComponent([]() { return processor; });
				scene->addRoute(nextRouteName, [](Packet<>* packet) {
					processor.next(packet);
				});
				scene->addRoute(errorRouteName, [](Packet<>* packet) {
					processor.error(packet);
				});
				scene->addRoute(completedRouteName, [](Packet<>* packet) {
					processor.complete(packet);
				});
			}
		};
		*/
	}
};
