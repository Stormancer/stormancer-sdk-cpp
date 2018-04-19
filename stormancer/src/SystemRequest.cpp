#include "stormancer/stdafx.h"
#include "stormancer/SystemRequest.h"

namespace Stormancer
{
	SystemRequest::SystemRequest(byte msgId, pplx::task_completion_event<Packet_ptr> tce)
		: tce(tce)
		, _msgId(msgId)
	{
	}

	SystemRequest::~SystemRequest()
	{
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
