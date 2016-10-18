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
		rxcpp::observable<Packetisp_ptr> rpc(const std::string route, std::function<void(bytestream*)> writer, PacketPriority priority);
		void addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContext_ptr)> handler, bool ordered);
		uint16 pendingRequests();
		void cancelAll(const char* reason);

		pplx::task<void> rpcVoid_with_writer(std::string procedure, std::function<void(Stormancer::bytestream*)> writer);

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
