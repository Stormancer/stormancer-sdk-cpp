#include "stormancer.h"

namespace Stormancer
{
	RpcService::RpcService(Scene* scene)
		: _scene(scene)
	{
	}

	RpcService::~RpcService()
	{
	}

	rxcpp::observable<Packetisp_ptr> RpcService::rpc(std::string route, Action<bytestream*> writer, PacketPriority priority)
	{
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([this, writer, route, priority](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto rr = _scene->remoteRoutes();
			Route_ptr relevantRoute;

			for (auto r : rr)
			{
				if (r->name == route)
				{
					relevantRoute = r;
					break;
				}
			}

			if (!relevantRoute)
			{
				throw std::runtime_error("The target route does not exist on the remote host.");
			}

			if (relevantRoute->metadata()[RpcClientPlugin::pluginName] != RpcClientPlugin::version)
			{
				throw std::runtime_error(std::string("The target remote route does not support the plugin RPC version ") + RpcClientPlugin::version);
			}

			auto request = new RpcRequest(subscriber);
			auto id = reserveId();

			if (!(mapContains(_pendingRequests, id)))
			{
				_pendingRequests[id] = request;
				_scene->sendPacket(route, [id, writer](bytestream* bs) {
					*bs << id;
					writer(bs);
				}, priority, PacketReliability::RELIABLE_ORDERED);
			}

			subscriber.add([this, id]() {
				_scene->sendPacket(RpcClientPlugin::cancellationRouteName, [this, id](bytestream* bs) {
					*bs << id;
				});
				if (mapContains(_pendingRequests, id))
				{
					_pendingRequests.erase(id);
				}
			});
		});
		return observable.as_dynamic();
	}

	uint16 RpcService::reserveId()
	{
		_mutex.lock();

		return 0;

		_mutex.unlock();
	}
};
