#include "stormancer.h"

namespace Stormancer
{
	const std::string RpcClientPlugin::pluginName = "stormancer.plugins.rpc";
	const std::string RpcClientPlugin::serviceName = "rpcService";
	const std::string RpcClientPlugin::version = "1.1.0";
	const std::string RpcClientPlugin::nextRouteName = "stormancer.rpc.next";
	const std::string RpcClientPlugin::errorRouteName = "stormancer.rpc.error";
	const std::string RpcClientPlugin::completeRouteName = "stormancer.rpc.completed";
	const std::string RpcClientPlugin::cancellationRouteName = "stormancer.rpc.cancel";

	void RpcClientPlugin::build(PluginBuildContext ctx)
	{
		ctx.sceneCreated += [this](Scene* scene) {
			auto rpcParams = scene->getHostMetadata(pluginName);

			if (rpcParams == version)
			{
				auto processor = new RpcService(scene);
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
		};

		ctx.sceneDisconnected += [this](Scene* scene) {
			auto processor = (RpcService*)scene->getComponent(RpcClientPlugin::serviceName);
			processor->disconnected();
		};
	}
};
