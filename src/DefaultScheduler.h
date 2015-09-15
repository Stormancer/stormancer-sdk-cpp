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
		STORMANCER_DLL_API DefaultScheduler();
		STORMANCER_DLL_API virtual ~DefaultScheduler();
		
	public:
		/// Schedule a periodic task on the scheculder
		STORMANCER_DLL_API std::shared_ptr<Subscription> schedulePeriodic(int delay, Action<> action);
	};
};
