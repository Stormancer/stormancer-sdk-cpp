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

	IObservable<Packetisp_ptr>* RpcService::rpc(const char* route, std::function<void(bytestream*)> writer, PacketPriority priority)
	{
		if (!_scene)
		{
			ILogger::instance()->log(LogLevel::Error, "rpc.rpc", "Invalid scene", "");
			throw std::runtime_error("The scene ptr is invalid");
		}

		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([this, writer, route, priority](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			auto rr = _scene->remoteRoutes();
			Route_ptr relevantRoute;

			auto sz = rr.Size();
			for (uint32 i = 0; i < sz; ++i)
			{
				auto r = rr[i];
				if (strcmp(r->name(), route) == 0)
				{
					relevantRoute = r;
					break;
				}
			}

			if (!relevantRoute)
			{
				ILogger::instance()->log(LogLevel::Error, "rpc.rpc", "The target route does not exist on the remote host.", "");
				throw std::runtime_error("The target route does not exist on the remote host.");
			}

			if (relevantRoute->metadata()[RpcPlugin::pluginName] != RpcPlugin::version)
			{
				auto errorMsg = std::string("The target remote route does not support the plugin RPC version ") + RpcPlugin::version;
				ILogger::instance()->log(LogLevel::Error, "rpc.rpc", errorMsg.c_str(), "");
				throw std::runtime_error(errorMsg);
			}

			RpcRequest_ptr request(new RpcRequest(subscriber));
			auto id = reserveId();
			request->id = id;

			{
				std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
				_pendingRequests[id] = request;
			}

			auto result = _scene->sendPacket(route, [id, writer](bytestream* bs) {
				*bs << id;
				writer(bs);
			}, priority, PacketReliability::RELIABLE_ORDERED);
			if (!result->success())
			{
				ILogger::instance()->log(LogLevel::Error, "rpc.rpc", "sendPacket failed", result->reason());
			}

			subscriber.add([this, request]() {
				if (!request->hasCompleted)
				{
					if (_scene->connectionState() == ConnectionState::Connected)
					{
						_scene->sendPacket(RpcPlugin::cancellationRouteName, [this, request](bytestream* bs) {
							*bs << request->id;
						});
					}
					eraseRequest(request->id);
				}
			});
		});

		return new Observable<Packetisp_ptr>(observable.as_dynamic());
	}

	uint16 RpcService::pendingRequests()
	{
		uint16 size = 0;
		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			size = (uint16)_pendingRequests.size();
		}
		return size;
	}

	void RpcService::addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, bool ordered)
	{
		if (!_scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		try
		{
			stringMap rpcMetadatas{ { RpcPlugin::pluginName, RpcPlugin::version } };

			_scene->addRoute(route, [this, handler, ordered](Packetisp_ptr p) {
				uint16 id = 0;
				*p->stream >> id;
				pplx::cancellation_token_source cts;
				RpcRequestContext_ptr ctx;
				_runningRequestsMutex.lock();
				if (!mapContains(_runningRequests, id))
				{
					_runningRequests[id] = cts;
					ctx = RpcRequestContext_ptr(new RpcRequestContext<IScenePeer>(p->connection, _scene, id, ordered, p->stream, cts.get_token()));
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
			}, &rpcMetadatas);
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

		std::lock_guard<std::mutex> lock(_pendingRequestsMutex);

		static uint16 id = 0;

		while (i <= 0xffff)
		{
			i++;
			id++;

			if (!mapContains(_pendingRequests, id))
			{
				return id;
				break;
			}
		}

		throw std::overflow_error("Unable to create a new RPC request: Too many pending requests.");
	}

	RpcRequest_ptr RpcService::getPendingRequest(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;

		RpcRequest_ptr request;

		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			if (mapContains(_pendingRequests, id))
			{
				request = _pendingRequests[id];
			}
		}

		if (!request)
		{
			auto idstr = std::to_string(id);
			ILogger::instance()->log(LogLevel::Warn, "RpcService", "Pending RPC request not found", idstr.c_str());
		}

		return request;
	}

	void RpcService::eraseRequest(uint16 requestId)
	{
		std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
		auto idstr = std::to_string(requestId);
		ILogger::instance()->log(LogLevel::Trace, "RpcService::complete", "Erasing RPC", idstr.c_str());
		_pendingRequests.erase(requestId);
	}

	void RpcService::next(Packetisp_ptr packet)
	{
		auto request = getPendingRequest(packet);
		if (request)
		{
#ifdef STORMANCER_LOG_RPC
			auto idstr = std::to_string(request->id);
			ILogger::instance()->log(LogLevel::Trace, "RpcService::next", "RPC next", idstr.c_str());
#endif

			request->observer.on_next(packet);
			if (!request->task.is_done())
			{
				request->tce.set();
			}
		}
	}

	void RpcService::error(Packetisp_ptr packet)
	{
		auto request = getPendingRequest(packet);
		if (request)
		{
#ifdef STORMANCER_LOG_RPC
			auto idstr = std::to_string(request->id);
			ILogger::instance()->log(LogLevel::Trace, "RpcService::error", "RPC error", idstr.c_str());
#endif

			request->hasCompleted = true;

			{
				std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
				_pendingRequests.erase(request->id);
			}

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
#ifdef STORMANCER_LOG_RPC
			auto idstr = std::to_string(request->id);
			ILogger::instance()->log(LogLevel::Trace, "RpcService::complete", "RPC complete", idstr.c_str());
			std::string messageSentStr = std::string() + "messageSent == " + (messageSent ? "true" : "false");
			ILogger::instance()->log(LogLevel::Trace, "RpcService::complete", messageSentStr.c_str(), idstr.c_str());
#endif

			request->hasCompleted = true;

			if (messageSent)
			{
				request->task.then([this, request](pplx::task<void> t) {
					eraseRequest(request->id);
					request->observer.on_completed();
				});
			}
			else
			{
				eraseRequest(request->id);
				request->observer.on_completed();
			}
		}
	}

	void RpcService::cancel(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;

#ifdef STORMANCER_LOG_RPC
		auto idstr = std::to_string(id);
		ILogger::instance()->log(LogLevel::Trace, "RpcService::cancel", "RPC cancel", idstr.c_str());
#endif

		_runningRequestsMutex.lock();
		if (mapContains(_runningRequests, id))
		{
			auto cts = _runningRequests[id];
			_runningRequests.erase(id);
			cts.cancel();
		}
		_runningRequestsMutex.unlock();
	}

	void RpcService::cancelAll(const char* reason)
	{
		{
			std::lock_guard<std::mutex> lg(_runningRequestsMutex);
			for (auto pair : _runningRequests)
			{
				pair.second.cancel();
			}
			_runningRequests.clear();
		}

		std::map<uint16, RpcRequest_ptr> pendingRequestsCopy;

		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			pendingRequestsCopy = _pendingRequests;
		}

		for (auto pair : pendingRequestsCopy)
		{
			if (!pair.second->hasCompleted)
			{
				pair.second->observer.on_error(std::make_exception_ptr<std::runtime_error>(std::runtime_error(reason)));
			}
		}

		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			_pendingRequests.clear();
		}
	}


	pplx::task<std::shared_ptr<Stormancer::Result<>>> RpcService::rpcVoid_with_writer( std::string procedure, std::function<void(Stormancer::bytestream*)> writer)
	{
		pplx::task_completion_event<std::shared_ptr<Stormancer::Result<>>> tce;
		std::shared_ptr<Stormancer::Result<>> result(new Stormancer::Result<>());

		auto observable = this->rpc(procedure.c_str(), [&writer](Stormancer::bytestream* stream) {
			writer(stream);
		}, PacketPriority::MEDIUM_PRIORITY);

		auto onNext = [tce, result](Stormancer::Packetisp_ptr packet) {			
			result->set();
			tce.set(result);
		};

		auto onError = std::function<void(const char*)>([tce, result, procedure](const char* error) {
			std::string msg = "Rpc::" + procedure;
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Error, "RpcHelpers", msg.c_str(), error);
			result->setError(1, error);
			tce.set(result);
		});

		auto onComplete = [observable]() {
			observable->destroy();
		};


		observable->subscribe(onNext, onError, onComplete);

		return pplx::create_task(tce);
	}
};




