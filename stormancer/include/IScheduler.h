#pragma once
#include "headers.h"
#include "Action.h"

namespace Stormancer
{
	/// A scheduler used to schedule network tasks
	class IScheduler
	{
	public:
		/// Schedule a periodic task on the scheculder
		virtual rxcpp::subscription schedulePeriodic(int delay, std::function<void()> funct) = 0;
	};
};
