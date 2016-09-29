#pragma once
#include "headers.h"
#include "RpcRequest.h"
#include "IRpcService.h"

namespace Stormancer
{
	class RpcService : public IRpcService
	{
	public:
		RpcService(Scene* scene, std::shared_ptr<IActionDispatcher> dispatcher);
		~RpcService();

	public:
		STORMANCER_DLL_API IObservable<Packetisp_ptr>* rpc(const std::string route, std::function<void(bytestream*)> writer, PacketPriority priority);
		STORMANCER_DLL_API void addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, bool ordered);
		STORMANCER_DLL_API uint16 pendingRequests();
		STORMANCER_DLL_API void cancelAll(const char* reason);

		STORMANCER_DLL_API  pplx::task<std::shared_ptr<Stormancer::Result<>>> rpcVoid_with_writer(std::string procedure, std::function<void(Stormancer::bytestream*)> writer);

		std::shared_ptr<IActionDispatcher> getDispatcher();
		



	public:
		void next(Packetisp_ptr packet);
		void error(Packetisp_ptr packet);
		void complete(Packetisp_ptr packet);
		void cancel(Packetisp_ptr packet);

	private:
		uint16 reserveId();
		RpcRequest_ptr getPendingRequest(Packetisp_ptr packet);
		void eraseRequest(uint16 requestId);

	private:
		std::shared_ptr<IActionDispatcher> _dispatcher;
		std::map<uint16, RpcRequest_ptr> _pendingRequests;
		std::mutex _pendingRequestsMutex;
		std::map<uint16, pplx::cancellation_token_source> _runningRequests;
		std::mutex _runningRequestsMutex;
		Scene* _scene;
	};
};
