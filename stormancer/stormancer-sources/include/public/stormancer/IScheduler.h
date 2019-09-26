#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Tasks.h"
#include "stormancer/Action.h"

namespace Stormancer
{
	class IActionDispatcher;

	/// A scheduler used to schedule network tasks
	class IScheduler
	{
	public:

		using clock_type = std::chrono::steady_clock;

		/// Schedule a cancellable periodic task on the scheculder
		/// \param delay milliseconds
		virtual void schedulePeriodic(int delay, std::function<void()> work, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// Schedule a cancellable periodic task on the scheculder
		/// \param delay milliseconds
		virtual void schedulePeriodic(int delay, std::function<void()> work, std::shared_ptr<IActionDispatcher> dispatcher, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// Schedule a single-shot task at a given time point
		/// <param name="dispatcher">The dispatcher on which the <c>work</c> will be run. Leave it to <c>nullptr</c> to use the default dispatcher.</param>
		virtual void schedule(clock_type::time_point when, std::function<void()> work, std::shared_ptr<IActionDispatcher> dispatcher = nullptr) = 0;
	};
}
