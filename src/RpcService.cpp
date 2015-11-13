#include "BoidsViewer.h"
#include "stormancer.h"

namespace Stormancer
{
	RpcService::RpcService(Scene_wptr scene)
		: _scene(scene)
	{
	}

	RpcService::~RpcService()
	{
	}
	/*
	rxcpp::observable<Packetisp_ptr> RpcService::rpc(const char* route2, Action<bytestream*> writer, PacketPriority priority)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		std::string route = route2;
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([this, scene, writer, route, priority](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto rr = scene->remoteRoutes();
			Route_ptr relevantRoute;
			
			auto sz = rr.Size();
			for (uint32 i = 0; i < sz; ++i)
			{
				auto r = rr[i];
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
			request->id = id;

			_pendingRequestsMutex.lock();
			_pendingRequests[id] = request;
			_pendingRequestsMutex.unlock();

			scene->sendPacket(route.c_str(), [id, writer](bytestream* bs) {
				*bs << id;
				writer(bs);
			}, priority, PacketReliability::RELIABLE_ORDERED);

			subscriber.add([this, scene, id]() {
				RpcRequest_ptr request;

				_pendingRequestsMutex.lock();
				if (mapContains(_pendingRequests, id))
				{
					request = _pendingRequests[id];
					_pendingRequests.erase(id);
				}
				_pendingRequestsMutex.unlock();

				if (request)
				{
					scene->sendPacket(RpcClientPlugin::cancellationRouteName, [this, id](bytestream* bs) {
						*bs << id;
					});
				}
			});
		});
		return observable.as_dynamic();
	}
	*/
	uint16 RpcService::pendingRequests()
	{
		_pendingRequestsMutex.lock();
		uint16 size = (uint16)_pendingRequests.size();
		_pendingRequestsMutex.unlock();
		return size;
	}

	void RpcService::addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContex_ptr)> handler, bool ordered)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		try
		{
			rakStringMap rpcMetadatas;
			rpcMetadatas.Set(RakNet::RakString(RpcClientPlugin::pluginName), RakNet::RakString(RpcClientPlugin::version));

			scene->addRoute(route, [this, scene, handler, ordered](Packetisp_ptr p) {
				uint16 id = 0;
				*p->stream >> id;
				pplx::cancellation_token_source cts;
				RpcRequestContex_ptr ctx;
				_runningRequestsMutex.lock();
				if (!mapContains(_runningRequests, id))
				{
					_runningRequests[id] = cts;
					ctx = RpcRequestContex_ptr(new RpcRequestContext<IScenePeer>(p->connection, scene, id, ordered, p->stream, cts.get_token()));
				}
				_runningRequestsMutex.unlock();

				if (ctx)
				{
					invokeWrapping(handler, ctx).then([this, id, ctx](pplx::task<void> t) {
						try
						{
							t.wait();
							bool requestFound = false;
							_runningRequestsMutex.lock();
							if (requestFound = mapContains(_runningRequests, id))
							{
								_runningRequests.erase(id);
							}
							_runningRequestsMutex.unlock();

							if (requestFound)
							{
								ctx->sendComplete();
							}
						}
						catch (const std::exception& e)
						{
							ctx->sendError(e.what());
						}
					});
				}
			}, rpcMetadatas);
		}
		catch (const std::exception& e)
		{
			std::runtime_error e2(e.what() + std::string("\nFailed to add procedure on the scene."));
			ILogger::instance()->log(e2);
			throw e2;
		}
	}

	uint16 RpcService::reserveId()
	{
		uint32 i = 0;
		uint16 idToReturn;
		bool found = false;

		_pendingRequestsMutex.lock();

		static uint16 id = 0;

		while (i <= 0xffff)
		{
			i++;
			id++;

			if (!mapContains(_pendingRequests, id))
			{
				idToReturn = id;
				found = true;
				break;
			}
		}

		_pendingRequestsMutex.unlock();

		if (found)
		{
			return idToReturn;
		}

		throw std::overflow_error("Unable to create a new RPC request: Too many pending requests.");
	}

	RpcRequest_ptr RpcService::getPendingRequest(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;

		RpcRequest_ptr request;

		_pendingRequestsMutex.lock();
		if (mapContains(_pendingRequests, id))
		{
			request = _pendingRequests[id];
		}
		_pendingRequestsMutex.unlock();

		return request;
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
		auto request = getPendingRequest(packet);
		if (request)
		{
			_pendingRequestsMutex.lock();
			_pendingRequests.erase(request->id);
			_pendingRequestsMutex.unlock();

			std::string buf;
			*packet->stream >> buf;
			msgpack::unpacked result;
			msgpack::unpack(result, buf.data(), buf.size());
			msgpack::object deserialized = result.get();
			std::string msg;
			deserialized.convert(&msg);
			request->observer.on_error(std::make_exception_ptr(std::runtime_error(msg)));
		}
	}

	void RpcService::complete(Packetisp_ptr packet)
	{
		byte b = 0;
		*packet->stream >> b;
		bool messageSent = (b != 0);

		auto request = getPendingRequest(packet);
		if (request)
		{
			_pendingRequestsMutex.lock();
			ILogger::instance()->log(Stormancer::LogLevel::Debug, "", "COMPLETE LOCKED", "");
			_pendingRequests.erase(request->id);
			_pendingRequestsMutex.unlock();
			ILogger::instance()->log(Stormancer::LogLevel::Debug, "", "COMPLETE UNLOCKED", "");

			if (messageSent)
			{
				request->task.then([request](pplx::task<void> t) {
					request->observer.on_completed();
				});
			}
			else
			{
				request->observer.on_completed();
			}
		}
	}

	void RpcService::cancel(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;

		_runningRequestsMutex.lock();
		if (mapContains(_runningRequests, id))
		{
			auto cts = _runningRequests[id];
			_runningRequests.erase(id);
			cts.cancel();
		}
		_runningRequestsMutex.unlock();
	}

	void RpcService::disconnected()
	{
		_runningRequestsMutex.lock();
		for (auto pair : _runningRequests)
		{
			pair.second.cancel();
		}
		_runningRequests.clear();
		_runningRequestsMutex.unlock();
	}
};
