#pragma once

#include "headers.h"
#include "RpcRequestContext.h"
#include "RpcRequest.h"
#include "Scene.h"
#include "IActionDispatcher.h"
#include "PacketPriority.h"

namespace Stormancer
{
	class RpcPlugin;

	class RpcService
	{
		friend class RpcPlugin;

		using Writer = std::function<void(bytestream*)>;

		template<typename TOutput>
		using Deserializer = std::function<TOutput(Packetisp_ptr)>;

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
		/// \param writer User function for serializing data to send. Use function (or lambda) of type : std::function<void(bytestream*)>.
		/// \param deserializer User function for deserializing data. Use function (or lambda) of type : std::function<TOutput(Packetisp_ptr)>.
		/// Call an operation on the server and get the success and the response.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation and the server response.
		template<typename TOutput>
		pplx::task<TOutput> rpcWriter(const std::string& procedure, const Writer& writer, const Deserializer<TOutput>& deserializer)
		{
			pplx::task_completion_event<TOutput> tce;

			auto observable = rpc_observable(procedure, writer, PacketPriority::MEDIUM_PRIORITY);

			auto onNext = [tce, deserializer](Packetisp_ptr packet) {
				RpcService::internalOnNext<TOutput>(tce, packet, deserializer); // this is used for managing TOutput = void
			};

			auto onError = [tce, procedure](const std::exception_ptr error) {
				ILogger::instance()->log(LogLevel::Trace, "RpcHelpers", "An exception occurred during the rpc " + procedure);
				tce.set_exception(error);
			};

			std::function<void()> onComplete = [tce]() {
				RpcService::internalOnComplete(tce); // this is used for managing TOutput = void
			};

			observable.subscribe(onNext, onError, onComplete);

			return pplx::create_task(tce, pplx::task_options(getDispatcher()));
		}

		/// RPC with writer function.
		/// \param procedure The procedure name.
		/// \param writer User function for serializing data to send. Use function (or lambda) of type : std::function<void(bytestream*)>.
		/// Call an operation on the server and get the success.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation.
		virtual pplx::task<void> rpcWriter(const std::string& procedure, const Writer& writer);

		/// RPC with auto-serialization.
		/// \param procedure The procedure name.
		/// \param args Variable number of serializable arguments to send, or Writer and Deserializer<TOutput>.
		/// Call an operation on the server and get the success and the response.
		/// \return pplx::task<TOutput> A task which exposes the success of the operation and the server response.
		template<typename TOutput, typename... TInputs>
		pplx::task<TOutput> rpc(const std::string& procedure, TInputs const&... args)
		{
			return rpcWriter<TOutput>(procedure, [&](bytestream* stream) {
				RpcService::pack(stream, args...);
			}, [](Packetisp_ptr packet) {
				return RpcService::internalDeserializer<TOutput>(packet);
			});
		}

#pragma endregion

	private:

#pragma region private_methods

		void next(Packetisp_ptr packet);
		void error(Packetisp_ptr packet);
		void complete(Packetisp_ptr packet);
		void cancel(Packetisp_ptr packet);

		static void pack(bytestream*)
		{
		}

		template<typename T, typename... Args>
		static void pack(bytestream* s, T value, Args... args)
		{
			msgpack::pack(s, value);
			RpcService::pack(s, args...);
		}

		template<typename TOutput>
		static void internalOnNext(pplx::task_completion_event<TOutput> tce, Packetisp_ptr packet, const Deserializer<TOutput>& deserializer)
		{
			TOutput data = deserializer(packet);
			tce.set(data);
		}

		template<typename TOutput>
		static void internalOnComplete(pplx::task_completion_event<TOutput>)
		{
		}

		template<typename TOutput>
		static TOutput internalDeserializer(Packetisp_ptr packet)
		{
			std::string buffer;
			*packet->stream >> buffer;
			msgpack::unpacked unp;
			msgpack::unpack(unp, buffer.data(), buffer.size());
			TOutput result;
			unp.get().convert(&result);
			return result;
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

#pragma endregion
	};

	template<>
	inline void RpcService::internalOnNext(pplx::task_completion_event<void>, Packetisp_ptr, const Deserializer<void>&)
	{
	}

	template<>
	inline void RpcService::internalOnComplete(pplx::task_completion_event<void> tce)
	{
		tce.set();
	}

	template<>
	inline void RpcService::internalDeserializer(Packetisp_ptr packet)
	{
	}
};
