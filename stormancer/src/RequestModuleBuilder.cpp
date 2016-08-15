#include "stormancer.h"

namespace Stormancer
{
	RequestModuleBuilder::RequestModuleBuilder(std::function<void(byte, std::function<pplx::task<void>(RequestContext*)>)> addHandler)
		: _addHandler(addHandler)
	{
	}

	RequestModuleBuilder::~RequestModuleBuilder()
	{
	}

	void RequestModuleBuilder::service(byte msgId, std::function<pplx::task<void>(RequestContext*)> handler)
	{
		_addHandler(msgId, handler);
	}
};
