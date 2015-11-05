#include "stormancer.h"

namespace Stormancer
{
	const char* RpcClientPlugin::pluginName = "stormancer.plugins.rpc";
	const char* RpcClientPlugin::serviceName = "rpcService";
	const char* RpcClientPlugin::version = "1.1.0";
	const char* RpcClientPlugin::nextRouteName = "stormancer.rpc.next";
	const char* RpcClientPlugin::errorRouteName = "stormancer.rpc.error";
	const char* RpcClientPlugin::completeRouteName = "stormancer.rpc.completed";
	const char* RpcClientPlugin::cancellationRouteName = "stormancer.rpc.cancel";

	void RpcClientPlugin::build(PluginBuildContext& ctx)
	{
		ctx.sceneCreated += [this](Scene_wptr scene_wptr) {
			auto scene = scene_wptr.lock();
			if (scene)
			{
				auto rpcParams = scene->getHostMetadata(pluginName);

				if (rpcParams == version)
				{
					auto processor = new RpcService(scene_wptr);
					scene->registerComponent(RpcClientPlugin::serviceName, [processor]() { return (void*)processor; });
					scene->addRoute(RpcClientPlugin::nextRouteName, [processor](Packetisp_ptr p) {
						processor->next(p);
					});
					scene->addRoute(RpcClientPlugin::cancellationRouteName, [processor](Packetisp_ptr p) {
						processor->cancel(p);
					});
					scene->addRoute(RpcClientPlugin::errorRouteName, [processor](Packetisp_ptr p) {
						processor->error(p);
					});
					scene->addRoute(RpcClientPlugin::completeRouteName, [processor](Packetisp_ptr p) {
						processor->complete(p);
					});
				}
			}
		};

		ctx.sceneDisconnected += [this](Scene_wptr scene_wptr) {
			auto scene = scene_wptr.lock();
			if (scene)
			{
				auto processor = (RpcService*)scene->getComponent(RpcClientPlugin::serviceName);
				processor->disconnected();
			}
		};
	}
};
