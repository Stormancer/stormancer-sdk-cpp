#include "stormancer/stdafx.h"
#include "stormancer/SystemRequest.h"

namespace Stormancer
{
	SystemRequest::SystemRequest(byte msgId, uint16 id, pplx::cancellation_token ct)
		: _ct(ct)
		, _msgId(msgId)
		, _id(id)
	{
	}

	byte SystemRequest::operation() const
	{
		return _msgId;
	}

	uint16 SystemRequest::getId() const
	{
		return _id;
	}

	bool SystemRequest::completed() const
	{
		return _complete;
	}

	bool SystemRequest::trySet(Packet_ptr packet)
	{
		if (!_complete)
		{
			bool shouldSet = false;
			{
				std::lock_guard<std::recursive_mutex> lg(_mutex);
				if (!_complete)
				{
					_complete = true;
					shouldSet = true;
				}
			}
			if (shouldSet)
			{
				_tce.set(packet);
				return true;
			}
		}
		return false;
	}

	bool SystemRequest::trySetException(const std::exception& ex)
	{
		if (!_complete)
		{
			bool shouldSet = false;
			{
				std::lock_guard<std::recursive_mutex> lg(_mutex);
				if (!_complete)
				{
					_complete = true;
					shouldSet = true;
				}
			}
			if (shouldSet)
			{
				_tce.set_exception(ex);
				return true;
			}
		}
		return false;
	}

	pplx::task<Packet_ptr> SystemRequest::getTask(pplx::task_options options) const
	{
		return pplx::create_task(_tce, options);
	}

	pplx::cancellation_token SystemRequest::getToken() const
	{
		return _ct;
	}

	void SystemRequest::setLastRefresh(std::chrono::system_clock::time_point lastRefresh)
	{
		_lastRefresh = lastRefresh;
	}

	std::chrono::system_clock::time_point SystemRequest::getLastRefresh() const
	{
		return _lastRefresh;
	}
}
