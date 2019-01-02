#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcRequest.h"

namespace Stormancer
{
	RpcRequest::RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer)
		: observer(observer)
	{
	}

	RpcRequest::~RpcRequest()
	{
		if (!hasCompleted)
		{
			observer.on_error(std::make_exception_ptr(pplx::task_canceled()));
			hasCompleted = true;
		}
	}
};
