#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcPlugin.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/RPC/RpcRequest.h"
#include "stormancer/Utilities/TaskUtilities.h"

namespace Stormancer
{
	RpcService::RpcService(std::weak_ptr<Scene> scene, std::shared_ptr<IActionDispatcher> dispatcher)
		: _dispatcher(dispatcher)
		, _scene(scene)
		, _logger(scene.lock()->dependencyResolver().resolve<ILogger>())
	{
	}

	RpcService::~RpcService()
	{
	}

	rxcpp::observable<Packetisp_ptr> RpcService::rpcObservable(const std::string& route, const StreamWriter& streamWriter, PacketPriority priority)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			throw std::runtime_error("The scene is invalid");
		}

		auto logger = _logger;
		auto wScene = _scene;
		auto observable = rxcpp::observable<>::create<Packetisp_ptr>([this, logger, wScene, route, streamWriter, priority](rxcpp::subscriber<Packetisp_ptr> subscriber)
		{
			auto scene = wScene.lock();

			if (!scene)
			{
				throw std::runtime_error("The scene is invalid");
			}

			auto rr = scene->remoteRoutes();
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
				std::string message = std::string() + "The target route '" + route + "' does not exist on the remote host.";
				logger->log(LogLevel::Error, "RpcService", message);
				throw std::runtime_error(message.c_str());
			}

			auto& metadata = relevantRoute->metadata();
			auto it = metadata.find(Stormancer::rpc::pluginName);
			if (it == metadata.end())
			{
				auto errorMsg = std::string() + "The target remote route '" + route + "' is not an RPC route.";
				logger->log(LogLevel::Error, "RpcService", errorMsg, route);
				throw std::runtime_error(errorMsg.c_str());
			}

			if (it->second != Stormancer::rpc::version)
			{
				auto errorMsg = std::string() + "The target remote route '" + route + "' does not support the plugin RPC version " + Stormancer::rpc::version;
				logger->log(LogLevel::Error, "RpcService", errorMsg.c_str(), route);
				throw std::runtime_error(errorMsg.c_str());
			}

			RpcRequest_ptr request(new RpcRequest(subscriber, route), [](RpcRequest* ptr) { delete ptr; });
			auto id = reserveId();
			request->id = id;

			{
				std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
				_pendingRequests.emplace(id, request);
			}

			try
			{
				scene->send(route, [id, streamWriter](obytestream& stream)
				{
					stream << id;
					if (streamWriter)
					{
						streamWriter(stream);
					}
				}, priority, PacketReliability::RELIABLE_ORDERED, _rpcServerChannelIdentifier);
			}
			catch (std::exception& ex)
			{
				subscriber.on_error(std::make_exception_ptr(ex));
				logger->log(LogLevel::Error, "RpcService", ("Failed to send rpc packet on route '" + route + "'").c_str(), ex.what());
				return;
			}

			std::weak_ptr<RpcRequest> wRequest = request;
			subscriber.add([this, logger, wScene, wRequest]()
			{
				if (auto request = wRequest.lock())
				{
					if (!request->hasCompleted)
					{
#ifdef STORMANCER_LOG_RPC
						auto idStr = std::to_string(request->id);
						_logger->log(LogLevel::Trace, "RpcService", "Cancel pending RPC", "id: " + idStr + ", route: " + request->route);
#endif

						if (auto scene = wScene.lock())
						{
							if (scene->getCurrentConnectionState() == ConnectionState::Connected)
							{
								try
								{
									scene->send(Stormancer::rpc::cancellationRouteName, [request](obytestream& stream)
									{
										stream << request->id;
									}, PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::RELIABLE_ORDERED, _rpcServerChannelIdentifier);
								}
								catch (std::exception& e)
								{
									logger->log(LogLevel::Error, "RpcService", "Failed to send rpc cancellation packet (route: " + request->route + ")", e.what());
								}
							}
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
		auto scene = _scene.lock();

		if (!scene)
		{
			throw std::runtime_error("The scene ptr is invalid");
		}

		try
		{
			std::map<std::string, std::string> rpcMetadatas{ { Stormancer::rpc::pluginName, Stormancer::rpc::version } };

			scene->addRoute(route, [this, handler, ordered](Packetisp_ptr p)
			{
				uint16 id = 0;
				p->stream >> id;
				pplx::cancellation_token_source cts;
				RpcRequestContext_ptr ctx;

				{
					std::lock_guard<std::mutex> lock(_runningRequestsMutex);
					if (_runningRequests.find(id) == _runningRequests.end())
					{
						_runningRequests.emplace(id, cts);
						ctx = std::make_shared<RpcRequestContext<IScenePeer>>(p->connection.get(), _scene, id, ordered, p->stream, cts.get_token());
					}
				}

				if (ctx)
				{
					invokeWrapping(handler, ctx)
						.then([this, id, ctx](pplx::task<void> t)
					{
						try
						{
							t.wait();
							bool requestFound = false;

							{
								std::lock_guard<std::mutex> lock(_runningRequestsMutex);
								auto it = _runningRequests.find(id);
								requestFound = it != _runningRequests.end();
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
			_logger->log(LogLevel::Error, "RpcService", "Failed to add procedure on the scene", ex.what());
			throw;
		}
	}

	uint16 RpcService::reserveId()
	{
		static uint16 id = 0;

		uint32 i = 0;

		std::lock_guard<std::mutex> lock(_pendingRequestsMutex);

		while (i <= 0xffff)
		{
			i++;
			id++;
			auto it = _pendingRequests.find(id);
			if (it == _pendingRequests.end())
			{
#ifdef STORMANCER_LOG_RPC
				auto idStr = std::to_string(id);
				_logger->log(LogLevel::Trace, "RpcService", "Create RPC", idStr.c_str());
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
		packet->stream >> id;

		RpcRequest_ptr request;

		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			auto it = _pendingRequests.find(id);
			if (it != _pendingRequests.end())
			{
				request = it->second;
			}
		}

		if (!request)
		{
			auto idstr = std::to_string(id);
			_logger->log(LogLevel::Warn, "RpcService", "Pending RPC request not found", idstr.c_str());
		}

		return request;
	}

	void RpcService::eraseRequest(uint16 requestId)
	{
		std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
#ifdef STORMANCER_LOG_RPC
		_logger->log(LogLevel::Trace, "RpcService", "Erase RPC", std::to_string(requestId));
#endif
		_pendingRequests.erase(requestId);
	}

	void RpcService::next(Packetisp_ptr packet)
	{
		auto request = getPendingRequest(packet);
		if (request)
		{
#ifdef STORMANCER_LOG_RPC
			auto idStr = std::to_string(request->id);
			_logger->log(LogLevel::Trace, "RpcService", "RPC next", "id: " + idStr + ", route: " + request->route);
#endif
			request->observer.on_next(packet);
			request->waitingForDataTce.set();
		}
	}

	void RpcService::error(Packetisp_ptr packet)
	{
		auto request = getPendingRequest(packet);
		if (request)
		{
#ifdef STORMANCER_LOG_RPC
			auto idStr = std::to_string(request->id);
			_logger->log(LogLevel::Trace, "RpcService", "RPC error", "id: " + idStr + ", route: " + request->route);
#endif
			request->hasCompleted = true;

			std::string msg = _serializer.deserializeOne<std::string>(packet->stream);
			if (msg.empty())
			{
				msg = "RPC failed (An unknown error occured on the server)";
			}

			request->observer.on_error(std::make_exception_ptr(std::runtime_error(msg.c_str())));
		}
	}

	void RpcService::complete(Packetisp_ptr packet)
	{
		byte b = 0;
		packet->stream >> b;
		bool messageSent = (b != 0);

		auto request = getPendingRequest(packet);
		if (request)
		{
#ifdef STORMANCER_LOG_RPC
			auto idStr = std::to_string(request->id);
			std::string messageSentStr = std::string() + "messageSent: " + (messageSent ? "true" : "false");
			_logger->log(LogLevel::Trace, "RpcService", "RPC complete", "id: " + idStr + ", route: " + request->route + ", " + messageSentStr);
#endif
			request->hasCompleted = true;

			pplx::task<void> waitingForDataTask = pplx::create_task(request->waitingForDataTce);

			if (!messageSent && !waitingForDataTask.is_done())
			{
				request->waitingForDataTce.set();
			}

			std::weak_ptr<RpcRequest> wRequest = request;
			waitingForDataTask
				.then([wRequest](pplx::task<void> t)
			{
				if (auto request = wRequest.lock())
				{
					request->observer.on_completed();
				}

				try
				{
					t.get();
				}
				catch (...)
				{
					// Do nothing
				}
			});
		}
	}

	void RpcService::cancel(Packetisp_ptr packet)
	{
		uint16 id = 0;
		packet->stream >> id;

#ifdef STORMANCER_LOG_RPC
		auto idStr = std::to_string(id);
		_logger->log(LogLevel::Trace, "RpcService", "cancel running RPC", "id: " + idStr);
#endif

		{
			std::lock_guard<std::mutex> lock(_runningRequestsMutex);
			auto it = _runningRequests.find(id);
			if (it != _runningRequests.end())
			{
				auto cts = it->second;
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

		std::unordered_map<uint16, RpcRequest_ptr> pendingRequestsCopy;

		{
			std::lock_guard<std::mutex> lock(_pendingRequestsMutex);
			pendingRequestsCopy = _pendingRequests;
			_pendingRequests.clear();
		}

		for (auto pair : pendingRequestsCopy)
		{
			if (!pair.second->hasCompleted)
			{
				pair.second->hasCompleted = true;
				pair.second->observer.on_error(std::make_exception_ptr<std::runtime_error>(std::runtime_error(reason.c_str())));
			}
		}
	}

	std::shared_ptr<IActionDispatcher> RpcService::getDispatcher()
	{
		return _dispatcher;
	}

	template<>
	pplx::task<Packetisp_ptr> RpcService::rpcImpl(rxcpp::observable<Packetisp_ptr> observable, const std::string& route, pplx::cancellation_token ct)
	{
		pplx::task_completion_event<Packetisp_ptr> tce;

		auto onNext = [tce](Packetisp_ptr packet)
		{
			tce.set(packet);
		};

		auto onComplete = []()
		{
		};

		return rpcInternal(observable, tce, route, onNext, onComplete, ct);
	}

	template<>
	pplx::task<void> RpcService::rpcImpl(rxcpp::observable<Packetisp_ptr> observable, const std::string& route, pplx::cancellation_token ct)
	{
		pplx::task_completion_event<void> tce;

		auto onNext = [](Packetisp_ptr packet)
		{
		};

		auto onComplete = [tce]()
		{
			tce.set();
		};

		return rpcInternal(observable, tce, route, onNext, onComplete, ct);
	}
};
