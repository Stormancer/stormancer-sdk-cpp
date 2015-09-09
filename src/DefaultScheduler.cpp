#include "stormancer.h"

namespace Stormancer
{
	DefaultScheduler::DefaultScheduler()
	{
	}

	DefaultScheduler::~DefaultScheduler()
	{
	}

	Subscription DefaultScheduler::schedulePeriodic(int delay, Action<> action)
	{
		if (delay <= 0)
		{
			throw new std::out_of_range("Scheduler periodic delay must be positive.");
		}

		auto now = rxcpp::schedulers::make_event_loop().create_worker().now();
		auto _syncClockSubscription = rxcpp::observable<>::interval(now, std::chrono::milliseconds(delay)).subscribe([this, action](int time) {
			action();
		});

		return Subscription(Action<>(std::function<void()>([_syncClockSubscription, action]() {
			_syncClockSubscription.unsubscribe();
		})));
	}
};
