#pragma once

#include "stormancer/headers.h"
#include "stormancer/RPC/RpcRequestContext.h"
#include "stormancer/RPC/RpcRequest.h"
#include "stormancer/Scene.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/PacketPriority.h"

namespace Stormancer
{
	class RpcPlugin;

	class RpcService
	{
		friend class RpcPlugin;

	public:

#pragma region public_methods

		RpcService(Scene* scene, std::shared_ptr<IActionDispatcher> dispatcher);

		~RpcService();

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
		pplx::task<TOutput> rpcWriter(const std::string& procedure, const Writer& writer, const Unwriter<TOutput>& unwriter)
		{
			pplx::task_completion_event<TOutput> tce;

			auto observable = rpc_observable(procedure, writer, PacketPriority::MEDIUM_PRIORITY);

			auto onNext = [=](Packetisp_ptr packet) {
				try
				{
					RpcService::internalOnNext<TOutput>(tce, packet, unwriter); // this is used for managing TOutput = void
				}
				catch (const std::exception& ex)
				{
					_logger->log(LogLevel::Trace, "RpcHelpers", "An exception occurred during the rpc response deserialization " + procedure);
					tce.set_exception(ex);
				}
			};

			auto onError = [=](std::exception_ptr error) {
				_logger->log(LogLevel::Trace, "RpcHelpers", "An exception occurred during the rpc " + procedure);
				tce.set_exception(error);
			};

			auto onComplete = [=]() {
				RpcService::internalOnComplete(tce); // this is used for managing TOutput = void
			};

			observable.subscribe(onNext, onError, onComplete);

			return pplx::create_task(tce, pplx::task_options(getDispatcher()));
		}

		/// RPC with writer function.
		/// \param procedure The procedure name.
		/// \param writer User function for serializing data to send. Use function (or lambda) of type : Writer.
		/// Call an operation on the server and get the success.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation.
		virtual pplx::task<void> rpcWriter(const std::string& procedure, const Writer& writer);

		/// RPC with auto-serialization.
		/// \param procedure The procedure name.
		/// \param args Variable number of serializable arguments to send, or Writer and Unwriter<TOutput>.
		/// Call an operation on the server and get the success and the response.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation and the server response.
		template<typename TOutput, typename... TInputs>
		pplx::task<TOutput> rpc(const std::string& procedure, TInputs const&... args)
		{
			return rpcWriter<TOutput>(procedure, [=, &args...](obytestream* stream) {
				_serializer.serialize(stream, args...);
			}, [=](ibytestream* stream) {
				return _serializer.deserializeOne<TOutput>(stream);
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
		RpcRequest_ptr getPendingRequest(Packetisp_ptr packet);
		void eraseRequest(uint16 requestId);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IActionDispatcher> _dispatcher;
		std::map<uint16, RpcRequest_ptr> _pendingRequests;
		std::mutex _pendingRequestsMutex;
		std::map<uint16, pplx::cancellation_token_source> _runningRequests;
		std::mutex _runningRequestsMutex;
		Scene* _scene;
		Serializer _serializer;
		const std::string _rpcServerChannelIdentifier = "RPC_server";
		ILogger_ptr _logger;

#pragma endregion
	};

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
