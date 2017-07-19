#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>
#include <functional>

namespace Stormancer
{

class TimerThread
{
public:

	using clock_type = std::chrono::steady_clock;

#pragma region public_methods

	TimerThread();

	~TimerThread();

	TimerThread(const TimerThread&) = delete;

	void schedule(std::function<void()> func, clock_type::time_point when);

#pragma endregion

private:

	struct QueueEntry
	{
		clock_type::time_point scheduledTime;
		std::function<void()> function;

		QueueEntry(clock_type::time_point t, std::function<void()> f) :
			scheduledTime(t),
			function(f)
		{}

		friend bool operator>(const QueueEntry& lhs, const QueueEntry& rhs)
		{
			return lhs.scheduledTime > rhs.scheduledTime;
		}
	};

	using func_queue_type = std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>>;

#pragma region private_methods

	void _threadLoop();

	void _stop(bool immediate = false);

#pragma endregion

#pragma region private_members

	std::mutex _mutex;
	std::condition_variable _cond;
	func_queue_type _taskQueue;
	bool _stopRequested = false;
	std::thread _thread;

#pragma endregion
};

}