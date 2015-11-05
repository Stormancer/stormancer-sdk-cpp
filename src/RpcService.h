#pragma once
#include "headers.h"
#include "RpcRequest.h"
#include "RpcRequestContext.h"

namespace Stormancer
{
	class RpcService
	{
	public:
		RpcService(Scene_wptr scene_wptr);
		~RpcService();

	public:
		STORMANCER_DLL_API rxcpp::observable<Packetisp_ptr> RpcService::rpc(const char* route, Action<bytestream*> writer, PacketPriority priority);
		STORMANCER_DLL_API uint16 pendingRequests();
		STORMANCER_DLL_API void addProcedure(const char* route, std::function<pplx::task<void>(RpcRequestContex_ptr)> handler, bool ordered);
		void next(Packetisp_ptr packet);
		void error(Packetisp_ptr packet);
		void complete(Packetisp_ptr packet);
		void cancel(Packetisp_ptr packet);
		void disconnected();

	private:
		uint16 reserveId();
		RpcRequest_ptr getPendingRequest(Packetisp_ptr packet);

	private:
		std::map<uint16, RpcRequest_ptr> _pendingRequests;
		std::mutex _pendingRequestsMutex;
		std::map<uint16, pplx::cancellation_token_source> _runningRequests;
		std::mutex _runningRequestsMutex;
		Scene_wptr _scene;
	};
};
