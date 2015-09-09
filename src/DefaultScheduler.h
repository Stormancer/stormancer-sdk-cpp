#pragma once
#include "headers.h"
#include "IScheduler.h"
#include "Subscription.h"

namespace Stormancer
{
	/// A scheduler used to schedule network tasks
	class DefaultScheduler : public IScheduler
	{
	public:
		DefaultScheduler();
		virtual ~DefaultScheduler();
		
	public:
		/// Schedule a periodic task on the scheculder
		Subscription schedulePeriodic(int delay, Action<> action);
	};
};
