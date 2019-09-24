#include "stormancer/stdafx.h"
#include "stormancer/DefaultScheduler.h"

namespace Stormancer
{
	void DefaultScheduler::schedulePeriodic(int delay, std::function<void()> work, pplx::cancellation_token ct)
	{
		schedulePeriodic(delay, work, nullptr, ct);
	}

	void DefaultScheduler::schedulePeriodic(int delay, std::function<void()> work, std::shared_ptr<IActionDispatcher> dispatcher, pplx::cancellation_token ct)
	{
		if (delay <= 0)
		{
			throw std::out_of_range("Scheduler periodic delay must be positive.");
		}

		std::weak_ptr<TimerThread> weakTimer(_timer);
		auto delayMs = std::chrono::milliseconds(delay);
		_timer->schedule([=]()
		{
			periodicFunc(work, ct, weakTimer, delayMs, dispatcher);
		}, clock_type::now(), dispatcher);
	}

	void DefaultScheduler::schedule(clock_type::time_point when, std::function<void()> work, std::shared_ptr<IActionDispatcher> dispatcher)
	{
		_timer->schedule(work, when, dispatcher);
	}

	void DefaultScheduler::periodicFunc(std::function<void()> work, pplx::cancellation_token ct, std::weak_ptr<TimerThread> weakTimer, std::chrono::milliseconds delay, std::shared_ptr<IActionDispatcher> dispatcher)
	{
		if (!ct.is_canceled())
		{
			work();

			auto timer = weakTimer.lock();
			if (timer)
			{
				timer->schedule([=]()
				{
					periodicFunc(work, ct, weakTimer, delay, dispatcher);
				}, clock_type::now() + delay, dispatcher);
			}
		}
	}
}
