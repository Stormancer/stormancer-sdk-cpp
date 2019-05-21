#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Tasks.h"
#include "stormancer/Action.h"

namespace Stormancer
{
	/// A scheduler used to schedule network tasks
	class IScheduler
	{
	public:

		using clock_type = std::chrono::steady_clock;

		/// Schedule a cancellable periodic task on the scheculder
		virtual void schedulePeriodic(int delay, std::function<void()> work, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// Schedule a single-shot task at a given time point
		virtual void schedule(clock_type::time_point when, std::function<void()> work) = 0;
	};
}
