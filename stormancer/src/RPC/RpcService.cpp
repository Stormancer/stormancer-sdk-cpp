#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcPlugin.h"
#include "stormancer/RPC/RpcService.h"
#include "stormancer/IActionDispatcher.h"

namespace Stormancer
{
	RpcService::RpcService(Scene* scene, std::shared_ptr<IActionDispatcher> dispatcher)
		: _dispatcher(dispatcher),
		_scene(scene)
	{
	}

	RpcService::~RpcService()
	{
	}

	rxcpp::observable<Packetisp_ptr> RpcService::rpc_observable(const std::string& route, const Writer& writer, PacketPriority priority)
	{
		if (!_scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([=](rxcpp::subscriber<Packetisp_ptr> subscriber) {
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			auto rr = _scene->remoteRoutes();
			Route_ptr relevantRoute;

			auto sz = rr.size();
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
				ILogger::instance()->log(LogLevel::Error, "RpcService", "The target route does not exist on the remote host.");
				throw std::runtime_error("The target route does not exist on the remote host.");
			}

			auto metadata = relevantRoute->metadata();

			if (!mapContains(metadata, RpcPlugin::pluginName))
			{
				auto errorMsg = std::string() + "The target remote route '" + route + "' is not an RPC route.";
				ILogger::instance()->log(LogLevel::Error, "RpcService", errorMsg, route);
				throw std::runtime_error(errorMsg);
			}
			
			if (metadata[RpcPlugin::pluginName] != RpcPlugin::version)
			{
				auto errorMsg = std::string() + "The target remote route '" + route + "' does not support the plugin RPC version " + RpcPlugin::version;
				ILogger::instance()->log(LogLevel::Error, "RpcService", errorMsg.c_str(), route);
				throw std::runtime_error(errorMsg);
			}

			RpcRequest_ptr request(new RpcRequest(subscriber));
			auto id = reserveId();
			request->id = id;

			{
				std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
				_pendingRequests[id] = request;
			}

			try
			{
				_scene->send(route, [=](obytestream* bs) {
					*bs << id;
					if (writer)
					{
						writer(bs);
					}
				}, priority, PacketReliability::RELIABLE_ORDERED, _rpcServerChannelIdentifier);
			}
			catch (std::exception& ex)
			{
				subscriber.on_error(std::make_exception_ptr(ex));
				ILogger::instance()->log(LogLevel::Error, "RpcService", ("Failed to send rpc packet on route " + route).c_str(), ex.what());
				return;
			}

			subscriber.add([=]() {
				if (!request->hasCompleted)
				{
#ifdef STORMANCER_LOG_RPC
					auto idStr = std::to_string(request->id);
					ILogger::instance()->log(LogLevel::Trace, "RpcService", "Cancel RPC", idStr.c_str());
#endif
					if (_scene && _scene->getCurrentConnectionState() == ConnectionState::Connected)
					{
						try
						{
							_scene->send(RpcPlugin::cancellationRouteName, [=](obytestream* bs)
							{
								*bs << request->id;
							}, PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::RELIABLE_ORDERED, _rpcServerChannelIdentifier);
						}
						catch (std::exception& e)
						{
							ILogger::instance()->log(LogLevel::Error, "RpcService", "Failed to send rpc cancellation packet", e.what());
						}
					}
					eraseRequest(request->id);
				}
			});
		});

		return observable.as_dynamic();
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

	void RpcService::addProcedure(const std::string& route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, MessageOriginFilter filter, bool ordered)
	{
		if (!_scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		try
		{
			std::map<std::string, std::string> rpcMetadatas{ { RpcPlugin::pluginName, RpcPlugin::version } };

			_scene->addRoute(route, [=](Packetisp_ptr p) {
				uint16 id = 0;
				*p->stream >> id;
				pplx::cancellation_token_source cts;
				RpcRequestContext_ptr ctx;

				{
					std::lock_guard<std::mutex> lock(_runningRequestsMutex);
					if (!mapContains(_runningRequests, id))
					{
						_runningRequests[id] = cts;
						ctx = std::make_shared<RpcRequestContext<IScenePeer>>(p->connection.get(), _scene, id, ordered, p->stream, cts.get_token());
					}
				}

				if (ctx)
				{
					invokeWrapping(handler, ctx).then([=](pplx::task<void> t) {
						try
						{
							t.wait();
							bool requestFound = false;

							{
								std::lock_guard<std::mutex> lock(_runningRequestsMutex);

								requestFound = mapContains(_runningRequests, id);
								if (requestFound)
								{
									_runningRequests.erase(id);
								}
							}

							if (requestFound)
							{
								ctx->sendComplete();
							}
						}
						catch (const std::exception& ex)
						{
							ctx->sendError(ex.what());
						}
					});
				}
			}, filter, rpcMetadatas);
		}
		catch (const std::exception& ex)
		{
			std::runtime_error e2(ex.what() + std::string("\nFailed to add procedure on the scene."));
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
#ifdef STORMANCER_LOG_RPC
				auto idStr = std::to_string(id);
				ILogger::instance()->log(LogLevel::Trace, "RpcService", "Create RPC", idStr.c_str());
#endif
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
#ifdef STORMANCER_LOG_RPC
		ILogger::instance()->log(LogLevel::Trace, "RpcService", "Erase RPC", std::to_string(requestId));
#endif
		_pendingRequests.erase(requestId);
	}

	void RpcService::next(Packetisp_ptr packet)
	{
		auto request = getPendingRequest(packet);
		if (request)
		{
#ifdef STORMANCER_LOG_RPC
			auto idstr = std::to_string(request->id);
			ILogger::instance()->log(LogLevel::Trace, "RpcService", "RPC next", idstr.c_str());
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
			ILogger::instance()->log(LogLevel::Trace, "RpcService", "RPC error", idstr.c_str());
#endif
			request->hasCompleted = true;

			eraseRequest(request->id);

			std::string msg = _serializer.deserializeOne<std::string>(packet->stream);
			
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
			ILogger::instance()->log(LogLevel::Trace, "RpcService", "RPC complete", idstr.c_str());
			std::string messageSentStr = std::string() + "messageSent == " + (messageSent ? "true" : "false");
			ILogger::instance()->log(LogLevel::Trace, "RpcService", messageSentStr.c_str(), idstr.c_str());
#endif
			request->hasCompleted = true;
			if (!messageSent)
			{
				if (!request->task.is_done())
				{
					request->tce.set();
				}
			}

			request->task.then([=](pplx::task<void> t) {
				eraseRequest(request->id);
				request->observer.on_completed();
			});
		}
	}

	void RpcService::cancel(Packetisp_ptr packet)
	{
		uint16 id = 0;
		*packet->stream >> id;

#ifdef STORMANCER_LOG_RPC
		auto idstr = std::to_string(id);
		ILogger::instance()->log(LogLevel::Trace, "RpcService", "cancel RPC", idstr.c_str());
#endif
		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			if (mapContains(_runningRequests, id))
			{
				auto cts = _runningRequests[id];
				_runningRequests.erase(id);
				cts.cancel();
			}
		}
	}

	void RpcService::cancelAll(const std::string& reason)
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

	std::shared_ptr<IActionDispatcher> RpcService::getDispatcher()
	{
		return _dispatcher;
	}

	pplx::task<void> RpcService::rpcWriter(const std::string& procedure, const Writer& writer)
	{
		pplx::task_completion_event<void> tce;

		auto observable = rpc_observable(procedure, writer, PacketPriority::MEDIUM_PRIORITY);

		observable.subscribe([](Packetisp_ptr packet) {
			// On next
		}, [=](std::exception_ptr exptr) {
			// On error
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				ILogger::instance()->log(LogLevel::Warn, "RpcHelpers", "An exception occurred during the rpc '" + procedure + "'", ex.what());
				tce.set_exception(ex);
			}
		}, [=]() {
			// On complete
			tce.set();
		});

		return pplx::create_task(tce, pplx::task_options(getDispatcher()));
	}
};
