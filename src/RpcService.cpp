#include "stormancer.h"
#include "RpcRequestContext.h"

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
				if (r->name() == route)
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

			RpcRequest_ptr request(new RpcRequest(subscriber));
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

	uint16 RpcService::pendingRequests()
	{
		return (uint16)_pendingRequests.size();
	}

	void RpcService::addProcedure(std::string route, std::function<pplx::task<void>(RpcRequestContex_ptr)> handler, bool ordered)
	{
		_scene->addRoute(route, [this, handler, ordered](Packetisp_ptr p) {
			uint16 id = 0;
			*p->stream >> id;
			pplx::cancellation_token_source cts;
			RpcRequestContex_ptr ctx(new RpcRequestContext<IScenePeer>(p->connection, _scene, id, ordered, p->stream, cts.get_token()));
			if (!mapContains(_runningRequests, id))
			{
				_runningRequests[id] = cts;
				handler(ctx).then([this, id, ctx](pplx::task<void> t) {
					try
					{
						t.wait();
						_runningRequests.erase(id);
						ctx->sendComplete();
					}
					catch (const std::exception& e)
					{
						ctx->sendError(e.what());
					}
				});
			}
		}, std::map<std::string, std::string> { { RpcClientPlugin::pluginName, RpcClientPlugin::version } });
	}

	uint16 RpcService::reserveId()
	{
		_mutex.lock();

		static uint16 id = 0;

		uint32 i = 0;
		while (i <= 0xffff)
		{
			bool found = false;
			if (!mapContains(_pendingRequests, id))
			{
				return id;
			}

			i++;
			id++;
		}

		_mutex.unlock();

		throw std::overflow_error("Unable to create a new RPC request: Too many pending requests.");
	}

	RpcRequest_ptr RpcService::getPendingRequest(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;

		if (mapContains(_pendingRequests, id))
		{
			return _pendingRequests[id];
		}
		else
		{
			return nullptr;
		}
	}

	void RpcService::next(Packetisp_ptr packet)
	{
		auto rq = getPendingRequest(packet);

		if (rq)
		{
			rq->receivedMsg++;
			rq->observer.on_next(packet);
			if (!rq->task.is_done())
			{
				rq->tce.set();
			}
		}
	}

	void RpcService::error(Packetisp_ptr packet)
	{
		auto rq = getPendingRequest(packet);
		if (rq)
		{
			std::string buf;
			*packet->stream >> buf;
			msgpack::unpacked result;
			msgpack::unpack(result, buf.data(), buf.size());
			msgpack::object deserialized = result.get();
			std::string msg;
			deserialized.convert(&msg);
			rq->observer.on_error(std::make_exception_ptr(std::runtime_error(msg)));
		}
	}

	void RpcService::complete(Packetisp_ptr packet)
	{
		byte b = 0;
		*packet->stream >> b;
		bool messageSent = (b != 0);

		auto rq = getPendingRequest(packet);
		if (rq)
		{
			if (messageSent)
			{
				rq->task.then([rq](pplx::task<void> t) {
					rq->observer.on_completed();
				});
			}
			else
			{
				rq->observer.on_completed();
			}
		}
	}

	void RpcService::cancel(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;
		
		if (mapContains(_runningRequests, id))
		{
			auto cts = _runningRequests[id];
			_runningRequests.erase(id);
			cts.cancel();
		}
	}

	void RpcService::disconnected()
	{
		for (auto pair : _runningRequests)
		{
			pair.second.cancel();
		}
	}
};
