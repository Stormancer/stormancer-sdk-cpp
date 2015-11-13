#pragma once
#include "headers.h"

namespace Stormancer
{
	class RpcRequest
	{
	public:
		STORMANCER_DLL_API RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer);
		STORMANCER_DLL_API ~RpcRequest();

	public:
		uint16 id = 0;
		int receivedMsg = 0;
		rxcpp::subscriber<Packetisp_ptr> observer;
		pplx::task_completion_event<void> tce;
		pplx::task<void> task;
	};

	using RpcRequest_ptr = std::shared_ptr<RpcRequest>;
};
