#pragma once

#include "stormancer/BuildConfig.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>
#include <functional>
#include <memory>

namespace Stormancer
{
	class IActionDispatcher;

	class TimerThread
	{
	public:

		using clock_type = std::chrono::steady_clock;

#pragma region public_methods

		static TimerThread& getInstance();

		TimerThread();

		~TimerThread();

		TimerThread(const TimerThread&) = delete;

		/// <summary>
		/// Set a function to be executed at the given time point.
		/// </summary>
		/// <param name="func">Function to be executed.</param>
		/// <param name="when">Time point when the function will be executed. This is not guaranteed to be 100% precise.</param>
		/// <param name="dispatcher">Dispatcher that <c>func</c> will be run on. Leave it to <c>nullptr</c> to use the default dispatcher.</param>
		void schedule(std::function<void()> func, clock_type::time_point when, std::shared_ptr<IActionDispatcher> dispatcher = nullptr);

#pragma endregion

	private:

		struct QueueEntry
		{
			clock_type::time_point scheduledTime;
			std::function<void()> function;
			std::shared_ptr<IActionDispatcher> dispatcher;

			QueueEntry(clock_type::time_point t, std::function<void()> f, std::shared_ptr<IActionDispatcher> d)
				: scheduledTime(t)
				, function(f)
				, dispatcher(d)
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

		static std::unique_ptr<TimerThread> _sInstance;

		std::mutex _mutex;
		std::condition_variable _cond;
		func_queue_type _taskQueue;
		bool _stopRequested = false;
		std::thread _thread;

#pragma endregion
	};
}
