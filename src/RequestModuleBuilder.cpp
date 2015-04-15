#include "stormancer.h"

namespace Stormancer
{
	RequestModuleBuilder::RequestModuleBuilder(function<void(byte, function<pplx::task<void>(RequestContext*)>)> addHandler)
		: _addHandler(addHandler)
	{
	}

	RequestModuleBuilder::~RequestModuleBuilder()
	{
	}

	void RequestModuleBuilder::service(byte msgId, function<pplx::task<void>(RequestContext*)> handler)
	{
		_addHandler(msgId, handler);
	}
};
