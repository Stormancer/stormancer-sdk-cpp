#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Tasks.h"
#include "stormancer/IScheduler.h"
#include "stormancer/TimerThread.h"

namespace Stormancer
{
	/// A scheduler used to schedule network tasks
	class DefaultScheduler : public IScheduler
	{
	public:

		STORMANCER_DLL_API DefaultScheduler() = default;

		STORMANCER_DLL_API virtual ~DefaultScheduler() = default;
		
		/// Schedule a cancellable periodic task on the scheculder
		STORMANCER_DLL_API void schedulePeriodic(int delay, std::function<void()> work, pplx::cancellation_token ct) override;

		STORMANCER_DLL_API void schedulePeriodic(int delay, std::function<void()> work, std::shared_ptr<IActionDispatcher> dispatcher, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		/// Schedule a single-shot task at a given time point
		STORMANCER_DLL_API void schedule(clock_type::time_point when, std::function<void()> work, std::shared_ptr<IActionDispatcher> dispatcher) override;

	private:
		
		static void periodicFunc(std::function<void()> work, pplx::cancellation_token ct, std::weak_ptr<TimerThread> weakTimer, std::chrono::milliseconds delay, std::shared_ptr<IActionDispatcher> dispatcher);

		std::shared_ptr<TimerThread> _timer = std::make_shared<TimerThread>();
	};
}
