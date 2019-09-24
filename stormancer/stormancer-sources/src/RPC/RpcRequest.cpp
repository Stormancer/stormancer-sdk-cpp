#include "stormancer/stdafx.h"
#include "stormancer/RPC/RpcRequest.h"

namespace Stormancer
{
	RpcRequest::RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer, std::string route)
		: observer(observer)
		, route(route)
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
}
