#include "stormancer.h"

namespace Stormancer
{
	void RpcClientPlugin::build(PluginBuildContext ctx)
	{
		ctx.sceneCreated += [this](Scene* scene) {
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				auto processor = new RpcService(scene);
				scene->registerComponent(RpcClientPlugin::serviceName, [processor]() { return (void*)processor; });
				scene->addRoute(RpcClientPlugin::nextRouteName, [processor](Packet_ptr p) {
					processor->next(p);
				});
				scene->addRoute(RpcClientPlugin::cancellationRouteName, [processor](Packet_ptr p) {
					processor->cancel(p);
				});
				scene->addRoute(RpcClientPlugin::errorRouteName, [processor](Packet_ptr p) {
					processor->error(p);
				});
				scene->addRoute(RpcClientPlugin::completeRouteName, [processor](Packet_ptr p) {
					processor->complete(p);
				});
			}
		};

		ctx.sceneDisconnected += [this](Scene* scene) {
			auto processor = (RpcService*)scene->getComponent(RpcClientPlugin::serviceName);
			processor->disconnected();
		};
	}
};
