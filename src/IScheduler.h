#pragma once
#include "headers.h"
#include "Action.h"
#include "Subscription.h"

namespace Stormancer
{
	/// A scheduler used to schedule network tasks
	class IScheduler
	{
	public:
		/// Schedule a periodic task on the scheculder
		virtual std::shared_ptr<Subscription> schedulePeriodic(int delay, Action<> action) = 0;
	};
};
