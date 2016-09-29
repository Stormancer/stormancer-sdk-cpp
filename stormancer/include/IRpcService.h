#pragma once
#include "headers.h"
#include "IObservable.h"
#include "RpcRequestContext.h"
#include "Scene.h"

namespace Stormancer
{
	class IRpcService
	{
	public:
		virtual IObservable<Packetisp_ptr>* rpc(const std::string route, std::function<void(bytestream*)> writer_ptr, PacketPriority priority) = 0;
		virtual void addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, bool ordered) = 0;
		virtual uint16 pendingRequests() = 0;
		virtual void cancelAll(const char* reason) = 0;

		
		virtual  pplx::task<std::shared_ptr<Stormancer::Result<>>> rpcVoid_with_writer(std::string procedure, std::function<void(Stormancer::bytestream*)> writer) = 0;

		virtual std::shared_ptr<IActionDispatcher> getDispatcher() = 0;


	public:

		template<typename TOutput>
		pplx::task<std::shared_ptr<Stormancer::Result<TOutput>>> rpc(std::string procedure, std::function<void(Stormancer::bytestream*)> writer, std::function<Stormancer::Result<TOutput>(Stormancer::Packetisp_ptr)> deserializer)
		{
			pplx::task_completion_event<std::shared_ptr<Stormancer::Result<TOutput>>> tce;
			std::shared_ptr<Stormancer::Result<TOutput>> result(new Stormancer::Result<TOutput>());

			auto observable = this->rpc(procedure.c_str(), [&writer](Stormancer::bytestream* stream) {
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

			return pplx::create_task(tce, pplx::task_options(getDispatcher()));
		}


		template<typename TInput>
		pplx::task<std::shared_ptr<Stormancer::Result<>>> rpcVoid(std::string procedure, TInput parameter)
		{
			return rpcVoid_with_writer(procedure, [&parameter](Stormancer::bytestream* stream) {
				msgpack::pack(stream, parameter);
			});
		}

		template<typename TInput, typename TOutput>
		pplx::task<std::shared_ptr<Stormancer::Result<TOutput>>> rpc(std::string procedure, TInput parameter)
		{
			return rpc<TOutput>(procedure, [&parameter](Stormancer::bytestream* stream) {
				msgpack::pack(stream, parameter);
			},
				[](Stormancer::Packetisp_ptr packet) {
				std::string buffer;
				*packet->stream >> buffer;
				msgpack::unpacked unp;
				msgpack::unpack(unp, buffer.data(), buffer.size());
				TOutput result;
				unp.get().convert(&result);
				return result;
			});
		}

		template<typename TOutput>
		pplx::task<std::shared_ptr<Stormancer::Result<TOutput>>> rpc(std::string procedure)
		{
			return rpc<TOutput>(procedure,
				[](Stormancer::bytestream* stream) {},
				[](Stormancer::Packetisp_ptr packet) {
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
