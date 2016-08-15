#include "stormancer.h"

namespace Stormancer
{
	RpcRequest::RpcRequest(rxcpp::subscriber<Packetisp_ptr>& observer)
		: observer(observer),
		task(pplx::create_task(tce))
	{
	}

	RpcRequest::~RpcRequest()
	{
	}
};
