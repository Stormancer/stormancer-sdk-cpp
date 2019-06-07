#include "stormancer/stdafx.h"
#include "stormancer/RequestModuleBuilder.h"

namespace Stormancer
{
	RequestModuleBuilder::RequestModuleBuilder(std::function<void(byte, std::function<pplx::task<void>(std::shared_ptr<RequestContext>)>)> addHandler)
		: _addHandler(addHandler)
	{
	}

	RequestModuleBuilder::~RequestModuleBuilder()
	{
	}

	void RequestModuleBuilder::service(byte msgId, std::function<pplx::task<void>(std::shared_ptr<RequestContext>)> handler)
	{
		_addHandler(msgId, handler);
	}
}
