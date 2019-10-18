#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	/// System request
	/// It will be canceled on delete if the internal tce is not set (tce::set or tce::set_exception)
	class SystemRequest
	{
	public:

#pragma region public_methods

		SystemRequest(byte msgId, uint16 id, pplx::cancellation_token ct);
		uint16 getId() const;
		byte operation() const;
		bool completed() const;
		bool trySet(Packet_ptr packet);

		// This methos needs to be templated to avoid slicing the exception.
		template<typename TEx>
		bool trySetException(TEx exception)
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
					_tce.set_exception(exception);
					return true;
				}
			}
			return false;
		}

		pplx::task<Packet_ptr> getTask(pplx::task_options options) const;
		pplx::cancellation_token getToken() const;
		void setLastRefresh(std::chrono::system_clock::time_point lastRefresh);
		std::chrono::system_clock::time_point getLastRefresh() const;

#pragma endregion

	private:

#pragma region private_members

		uint16 _id = 0;
		byte _msgId = 0;
		bool _complete = false;
		std::chrono::system_clock::time_point _lastRefresh;
		std::recursive_mutex _mutex;
		pplx::task_completion_event<Packet_ptr> _tce;
		pplx::cancellation_token _ct;

#pragma endregion
	};

	using SystemRequest_ptr = std::shared_ptr<SystemRequest>;
}
