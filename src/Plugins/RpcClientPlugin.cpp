
#include "Plugins/RpcClientPlugin.h"
#include "Helpers.h"
#include "Plugins/RpcService.h"

namespace Stormancer
{
	RpcClientPlugin::RpcClientPlugin()
	{
	}

	RpcClientPlugin::~RpcClientPlugin()
	{
	}

	void RpcClientPlugin::build(PluginBuildContext ctx)
	{
		ctx.sceneCreated << [&](Scene& scene) {
			auto rpcParams = scene.getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				auto processor = RpcService(scene);
				scene.registerComponent([]() { return processor; });
				scene.addRoute(nextRouteName, [](Packet2 packet) {
					processor.next(packet);
				});
				scene.addRoute(errorRouteName, [](Packet2 packet) {
					processor.error(packet);
				});
				scene.addRoute(completedRouteName, [](Packet2 packet) {
					processor.complete(packet);
				});
			}
		};
	}
};
