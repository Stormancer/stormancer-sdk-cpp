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
		rxcpp::observable<Packetisp_ptr> rpc_observable(const std::string& route, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY);

		/// RPC with writer and deserializer functions.
		/// \param procedure The procedure name.
		/// \param writer User function for serializing data to send. Use function (or lambda) of type Writer.
		/// \param deserializer User function for deserializing data. Use function (or lambda) of type Unwriter.
		/// Call an operation on the server and get the success and the response.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation and the server response.
		template<typename TOutput>
		pplx::task<TOutput> rpcWriter(const std::string& procedure, pplx::cancellation_token ct, const Writer& writer, const Unwriter<TOutput>& unwriter)
		{
			auto logger = _logger;

			pplx::task_completion_event<TOutput> tce;

			auto observable = rpc_observable(procedure, writer, PacketPriority::MEDIUM_PRIORITY);

			auto onNext = [logger, tce, procedure, unwriter](Packetisp_ptr packet)
			{
				try
				{
					RpcService::internalOnNext<TOutput>(tce, packet, unwriter); // this is used for managing TOutput = void
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Trace, "RpcHelpers", "An exception occurred during the rpc response deserialization " + procedure);
					tce.set_exception(ex);
				}
			};

			auto onError = [logger, tce, procedure](std::exception_ptr error)
			{
				logger->log(LogLevel::Trace, "RpcHelpers", "An exception occurred during the rpc " + procedure);
				tce.set_exception(error);
			};

			auto onComplete = [tce]()
			{
				RpcService::internalOnComplete(tce); // this is used for managing TOutput = void
			};

			auto subscription = observable.subscribe(onNext, onError, onComplete);
			
			if (ct.is_cancelable())
			{
				ct.register_callback([subscription]() {
					if (subscription.is_subscribed())
					{
						subscription.unsubscribe();
					}
				});
			}
			return pplx::create_task(tce, getDispatcher());
		}

		/// RPC with writer function.
		/// \param procedure The procedure name.
		/// \param writer User function for serializing data to send. Use function (or lambda) of type : Writer.
		/// Call an operation on the server and get the success.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation.
		virtual pplx::task<void> rpcWriter(const std::string& procedure, pplx::cancellation_token ct, const Writer& writer);

		/// RPC with auto-serialization.
		/// \param procedure The procedure name.
		/// \param args Variable number of serializable arguments to send, or Writer and Unwriter<TOutput>.
		/// Call an operation on the server and get the success and the response.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation and the server response.
		template<typename TOutput, typename... TInputs>
		pplx::task<TOutput> rpc(const std::string& procedure, TInputs const&... args)
		{
			auto serializer = _serializer;
			return rpcWriter<TOutput>(procedure, pplx::cancellation_token::none(), [serializer, &args...](obytestream* stream) {
				serializer.serialize(stream, args...);
			}, [serializer](ibytestream* stream) {
				return serializer.deserializeOne<TOutput>(stream);
			});
		}
		
		/// RPC with auto-serialization.
		/// \param procedure The procedure name.
		/// \param args Variable number of serializable arguments to send, or Writer and Unwriter<TOutput>.
		/// Call an operation on the server and get the success and the response.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation and the server response.
		template<typename TOutput, typename... TInputs>
		pplx::task<TOutput> rpc(const std::string& procedure, pplx::cancellation_token ct, TInputs const&... args)
		{
			auto serializer = _serializer;
			return rpcWriter<TOutput>(procedure,ct, [serializer, &args...](obytestream* stream) {
				serializer.serialize(stream, args...);
			}, [serializer](ibytestream* stream) {
				return serializer.deserializeOne<TOutput>(stream);
			});
		}
#pragma endregion

	private:

#pragma region private_methods

		void next(Packetisp_ptr packet);
		void error(Packetisp_ptr packet);
		void complete(Packetisp_ptr packet);
		void cancel(Packetisp_ptr packet);

		template<typename TOutput>
		static void internalOnNext(pplx::task_completion_event<TOutput> tce, Packetisp_ptr packet, const Unwriter<TOutput>& unwriter)
		{
			TOutput data = unwriter(packet->stream);
			tce.set(data);
		}

		template<typename TOutput>
		static void internalOnComplete(pplx::task_completion_event<TOutput>)
		{
			// Do nothing
		}

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

	// Specializations

	template<>
	inline void RpcService::internalOnNext(pplx::task_completion_event<void>, Packetisp_ptr, const Unwriter<void>&)
	{
		// Do nothing
	}

	template<>
	inline void RpcService::internalOnComplete(pplx::task_completion_event<void> tce)
	{
		tce.set();
	}
};
