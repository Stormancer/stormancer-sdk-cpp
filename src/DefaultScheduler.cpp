#include "stormancer.h"

namespace Stormancer
{
	DefaultScheduler::DefaultScheduler()
	{
	}

	DefaultScheduler::~DefaultScheduler()
	{
	}

	ISubscription* DefaultScheduler::schedulePeriodic(int delay, Action<> action)
	{
		if (delay <= 0)
		{
			throw std::out_of_range("Scheduler periodic delay must be positive.");
		}
		
		auto scheduler = rxcpp::schedulers::make_same_worker(rxcpp::schedulers::make_event_loop().create_worker());
		auto coordination = rxcpp::identity_one_worker(scheduler);

		auto _syncClockSubscription = rxcpp::observable<>::interval(std::chrono::milliseconds(delay), coordination).subscribe([action](int time) {
			action();
		});

		//auto unsubscribeAction = Action<>(std::function<void()>([_syncClockSubscription]() {
		//	_syncClockSubscription.unsubscribe();
		//}));
		
		return new Subscription([_syncClockSubscription]() {
			_syncClockSubscription.unsubscribe();
		});
	}
};
