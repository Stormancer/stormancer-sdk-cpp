#pragma once
#include "headers.h"

namespace Stormancer
{
	class RpcRequest
	{
	public:
		RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer);
		~RpcRequest();

	public:
		rxcpp::subscriber<Packetisp_ptr> observer;
		int receivedMsg;
		pplx::task_completion_event<void> tce;
		pplx::task<void> task;
	};
};
