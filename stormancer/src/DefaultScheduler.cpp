#include "stdafx.h"
#include "DefaultScheduler.h"

namespace Stormancer
{
	DefaultScheduler::DefaultScheduler()
	{
	}

	DefaultScheduler::~DefaultScheduler()
	{
	}

	void DefaultScheduler::schedulePeriodic(int delay, std::function<void()> work, pplx::cancellation_token ct)
	{
		if (delay <= 0)
		{
			throw std::out_of_range("Scheduler periodic delay must be positive.");
		}
		
		std::weak_ptr<TimerThread> weakTimer(_timer);
		auto delayMs = std::chrono::milliseconds(delay);
		_timer->schedule([=]()
		{
			periodicFunc(work, ct, weakTimer, delayMs);
		}, clock_type::now());
	}

	void DefaultScheduler::schedule(clock_type::time_point when, std::function<void()> work)
	{
		_timer->schedule(work, when);
	}

	void DefaultScheduler::periodicFunc(std::function<void()> work, pplx::cancellation_token ct, std::weak_ptr<TimerThread> weakTimer, std::chrono::milliseconds delay)
	{
		if (!ct.is_canceled())
		{
			work();

			auto timer = weakTimer.lock();
			if (timer)
			{
				timer->schedule([=]()
				{
					periodicFunc(work, ct, weakTimer, delay);
				}, clock_type::now() + delay);
			}
		}
	}
}
