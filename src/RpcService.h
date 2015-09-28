#pragma once
#include "headers.h"
#include "RpcRequest.h"

namespace Stormancer
{
	class RpcService
	{
	public:
		RpcService(Scene* scene);
		~RpcService();

	public:
		rxcpp::observable<Packetisp_ptr> RpcService::rpc(std::string route, Action<bytestream*> writer, PacketPriority priority);
		uint16 pendingRequests();
		void addProcedure(std::string route, std::function<pplx::task<void>(RpcRequestContex_ptr)> handler, bool ordered);
		void next(Packetisp_ptr packet);
		void error(Packetisp_ptr packet);
		void complete(Packetisp_ptr packet);
		void cancel(Packetisp_ptr packet);
		void disconnected();

	private:
		uint16 reserveId();
		RpcRequest_ptr getPendingRequest(Packetisp_ptr packet);

	private:
		std::mutex _mutex;
		std::map<uint16, RpcRequest_ptr> _pendingRequests;
		std::mutex _pendingRequestsMutex;
		std::map<uint16, pplx::cancellation_token_source> _runningRequests;
		std::mutex _runningRequestsMutex;
		Scene* _scene;
	};
};
