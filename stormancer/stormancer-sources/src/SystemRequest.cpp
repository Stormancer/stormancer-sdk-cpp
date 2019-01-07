#include "stormancer/stdafx.h"
#include "stormancer/SystemRequest.h"

namespace Stormancer
{
	SystemRequest::SystemRequest(byte msgId, pplx::task_completion_event<Packet_ptr> tce, pplx::cancellation_token ct)
		: tce(tce)
		, ct(ct)
		, _msgId(msgId)
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
			std::string message = "System request is not finished and has been deleted : id=" + std::to_string(_msgId);
			tce.set_exception(std::runtime_error(message.c_str()));
		}
	}

	byte SystemRequest::operation()
	{
		return _msgId;
	}
};
