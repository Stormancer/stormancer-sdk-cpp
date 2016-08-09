#pragma once
#include "IObservable.h"
#include "RpcRequestContext.h"
#include "Scene.h"

namespace Stormancer
{
	class IRpcService
	{
	public:
		virtual IObservable<Packetisp_ptr>* rpc(const char* route, std::function<void(bytestream*)> writer_ptr, PacketPriority priority) = 0;
		virtual void addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, bool ordered) = 0;
		virtual uint16 pendingRequests() = 0;
		virtual void cancelAll(const char* reason) = 0;


		template<typename TOutput>
		pplx::task<std::shared_ptr<Stormancer::Result<TOutput>>> rpc(Stormancer::IRpcService* rpc, std::string procedure, std::function<void(Stormancer::bytestream*)> writer, std::function<Stormancer::Result<TOutput>(Stormancer::Packetisp_ptr)> deserializer)
		{
			pplx::task_completion_event<std::shared_ptr<Stormancer::Result<TOutput>>> tce;
			std::shared_ptr<Stormancer::Result<TOutput>> result(new Stormancer::Result<TOutput>());

			auto observable = rpc->rpc(procedure.c_str(), [&writer](Stormancer::bytestream* stream) {
				writer(stream);
			}, PacketPriority::MEDIUM_PRIORITY);

			auto onNext = [tce, result, deserializer](Stormancer::Packetisp_ptr packet) {
				TOutput data = deserializer(packet).get();
				result->set(data);
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

		template<typename TInput>
		pplx::task<std::shared_ptr<Stormancer::Result<>>> rpcVoid(Stormancer::IRpcService* rpcService, std::string procedure, TInput parameter)
		{
			return rpcVoid_with_writer(rpcService, procedure, [&parameter](Stormancer::bytestream* stream) {
				msgpack::pack(stream, parameter);
			});
		}


		STORMANCER_DLL_API  pplx::task<std::shared_ptr<Stormancer::Result<>>> rpcVoid_with_writer(Stormancer::IRpcService* rpc, std::string procedure, std::function<void(Stormancer::bytestream*)> writer);


		template<typename TInput, typename TOutput>
		pplx::task<std::shared_ptr<Stormancer::Result<TOutput>>> rpc(Stormancer::IRpcService* rpcService, std::string procedure, TInput parameter)
		{
			return rpc<TOutput>(rpcService, procedure, [&parameter](Stormancer::bytestream* stream) {
				msgpack::pack(stream, parameter);
			}, [](Stormancer::Packetisp_ptr packet) {
				std::string buffer;
				*packet->stream >> buffer;
				msgpack::unpacked unp;
				msgpack::unpack(unp, buffer.data(), buffer.size());
				TOutput result;
				unp.get().convert(&result);
				return result;
			});
		}

		

	};
};
