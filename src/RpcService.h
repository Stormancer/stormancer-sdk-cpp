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

	private:
		uint16 reserveId();

	private:
		uint16 _currentRequestId = 0;
		std::mutex _mutex;
		std::map<uint16, RpcRequest*> _pendingRequests;
		std::mutex _pendingRequestsMutex;
		std::map<uint32, pplx::cancellation_token_source> _runningRequests;
		std::mutex _runningRequestsMutex;
		Scene* _scene;
	};
};
