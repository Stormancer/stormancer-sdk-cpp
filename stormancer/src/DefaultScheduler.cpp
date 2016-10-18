#include "stormancer.h"

namespace Stormancer
{
	DefaultScheduler::DefaultScheduler()
	{
	}

	DefaultScheduler::~DefaultScheduler()
	{
	}

	rxcpp::subscription DefaultScheduler::schedulePeriodic(int delay, std::function<void()> func)
	{
		if (delay <= 0)
		{
			throw std::out_of_range("Scheduler periodic delay must be positive.");
		}
		
		auto scheduler = rxcpp::schedulers::make_same_worker(rxcpp::schedulers::make_event_loop().create_worker());
		auto coordination = rxcpp::identity_one_worker(scheduler);

		return rxcpp::observable<>::interval(std::chrono::milliseconds(delay), coordination).subscribe([func](int time) {
			func();
		});

		
	}
};
