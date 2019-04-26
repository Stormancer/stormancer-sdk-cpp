#pragma once

#include "stormancer/BuildConfig.h"

#include <memory>
#include <string>
#include <unordered_map>

#include "stormancer/Tasks.h"
#include "rxcpp/rx.hpp"
#include "stormancer/RPC/RpcRequestContext.h"
#include "stormancer/Scene.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/MessageOriginFilter.h"
#include "stormancer/Packet.h"


namespace Stormancer
{
	class ILogger;
	class RpcPlugin;
	class RpcRequest;
	class RpcService
	{
		friend class RpcPlugin;

	public:

#pragma region public_methods

		RpcService(std::weak_ptr<Scene> scene, std::shared_ptr<IActionDispatcher> dispatcher);

		virtual ~RpcService();

		/// Add a procedure to execute when the server send an RPC
		void addProcedure(const std::string& route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, MessageOriginFilter filter = MessageOriginFilter::Host, bool ordered = false);

		uint16 pendingRequests();

		/// Cancel all RPCs
		void cancelAll(const std::string& reason);

		std::shared_ptr<IActionDispatcher> getDispatcher();

		/// Send an RPC and returns an observable
		rxcpp::observable<Packetisp_ptr> rpcObservable(const std::string& route, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY);

		template<typename TOut = void, typename... TIn>
		pplx::task<TOut> rpc(const std::string& route, pplx::cancellation_token ct, const TIn& ... args)
		{
			auto& serializer = _serializer;
			auto streamWriter = [&serializer, &args...](obytestream& stream)
			{
				serializer.serialize(stream, args...);
			};
			return rpcImpl<TOut>(rpcObservable(route, streamWriter), route, ct);
		}

		template<typename TOut = void, typename TStreamWriter>
		typename std::enable_if<std::is_convertible<TStreamWriter, StreamWriter>::value, pplx::task<TOut>>::type rpc(const std::string& route, pplx::cancellation_token ct, const TStreamWriter& streamWriter)
		{
			return rpcImpl<TOut>(rpcObservable(route, streamWriter), route);
		}

		template<typename TOut = void, typename... TIn>
		pplx::task<TOut> rpc(const std::string& route, const TIn& ... args)
		{
			return rpc<TOut, TIn...>(route, pplx::cancellation_token::none(), args...);
		}

		template<typename TOut = void, typename TStreamWriter>
		typename std::enable_if<std::is_convertible<TStreamWriter, StreamWriter>::value, pplx::task<TOut>>::type rpc(const std::string& route, const TStreamWriter& streamWriter)
		{
			return rpc<TOut, TStreamWriter>(route, pplx::cancellation_token::none(), streamWriter);
		}

#pragma endregion

	private:

#pragma region private_methods

		template<typename TOut>
		pplx::task<TOut> rpcImpl(rxcpp::observable<Packetisp_ptr> observable, const std::string& route, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			pplx::task_completion_event<TOut> tce;

			auto serializer = _serializer;
			auto onNext = [serializer, tce](Packetisp_ptr packet)
			{
				TOut out = serializer.deserializeOne<TOut>(packet->stream);
				tce.set(out);
			};

			auto onComplete = []()
			{
			};

			return rpcInternal(observable, tce, route, onNext, onComplete, ct);
		}

		template<typename TOut>
		pplx::task<TOut> rpcInternal(rxcpp::observable<Packetisp_ptr> observable, pplx::task_completion_event<TOut> tce, const std::string& route, const std::function<void(Packetisp_ptr)>& onNext, const std::function<void()>& onComplete, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			auto logger = _logger;
			auto onError = [logger, tce, route](std::exception_ptr error)
			{
				logger->log(LogLevel::Trace, "Rpc", "An exception occurred during the rpc '" + route + "'");
				tce.set_exception(error);
			};

			auto subscription = observable.subscribe(onNext, onError, onComplete);

			if (ct.is_cancelable())
			{
				ct.register_callback([subscription]()
				{
					if (subscription.is_subscribed())
					{
						subscription.unsubscribe();
					}
				});
			}

			return create_task(tce, getDispatcher());
		}

		void next(Packetisp_ptr packet);
		void error(Packetisp_ptr packet);
		void complete(Packetisp_ptr packet);
		void cancel(Packetisp_ptr packet);

		uint16 reserveId();
		std::shared_ptr<RpcRequest> getPendingRequest(Packetisp_ptr packet);
		void eraseRequest(uint16 requestId);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IActionDispatcher> _dispatcher;
		std::unordered_map<uint16, std::shared_ptr<RpcRequest>> _pendingRequests;
		std::mutex _pendingRequestsMutex;
		std::unordered_map<uint16, pplx::cancellation_token_source> _runningRequests;
		std::mutex _runningRequestsMutex;
		std::weak_ptr<Scene> _scene;
		Serializer _serializer;
		const std::string _rpcServerChannelIdentifier = "RPC_server";
		std::shared_ptr<Stormancer::ILogger> _logger;

#pragma endregion
	};

	template<>
	pplx::task<Packetisp_ptr> RpcService::rpcImpl(rxcpp::observable<Packetisp_ptr> observable, const std::string& route, pplx::cancellation_token ct);

	template<>
	pplx::task<void> RpcService::rpcImpl(rxcpp::observable<Packetisp_ptr> observable, const std::string& route, pplx::cancellation_token ct);
};
