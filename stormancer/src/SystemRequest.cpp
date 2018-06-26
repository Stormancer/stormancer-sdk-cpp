#include "stormancer/stdafx.h"
#include "stormancer/SystemRequest.h"

namespace Stormancer
{
	SystemRequest::SystemRequest(byte msgId, pplx::task_completion_event<Packet_ptr> tce, pplx::cancellation_token ct)
		: tce(tce)
		, _msgId(msgId)
		,ct(ct)
	{
	}

	SystemRequest::~SystemRequest()
	{
		if (ct != pplx::cancellation_token::none())
		{
			ct.deregister_callback(ct_registration);
		}
		if (!complete)
		{
			tce.set_exception(std::runtime_error("System request is not finished and has been deleted"));
		}
	}

	byte SystemRequest::operation()
	{
		return _msgId;
	}
};
