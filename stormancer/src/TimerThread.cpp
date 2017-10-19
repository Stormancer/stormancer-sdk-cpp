#include "stdafx.h"
#include "TimerThread.h"
#include <cassert>

namespace Stormancer
{

std::unique_ptr<TimerThread> TimerThread::_sInstance;

TimerThread& TimerThread::getInstance()
{
	if (!_sInstance)
	{
		_sInstance.reset(new TimerThread);
	}

	return *_sInstance;
}

TimerThread::TimerThread() :
	_thread(&TimerThread::_threadLoop, this)
{
}

TimerThread::~TimerThread()
{
	_stop(true);
}

void TimerThread::schedule(std::function<void()> func, clock_type::time_point when)
{
	assert(func);

	std::unique_lock<std::mutex> lock(_mutex);

	if (_stopRequested)
	{
		return;
	}

	// Do not notify the thread if there is a pending function that is scheduled to run earlier
	// than the new to-be-scheduled function
	bool notifyThread = true;
	if (!_taskQueue.empty() && _taskQueue.top().scheduledTime <= when)
	{
		notifyThread = false;
	}

	_taskQueue.emplace(when, func);
	if (notifyThread)
	{
		_cond.notify_one();
	}
}

void TimerThread::_stop(bool immediate)
{
	{
		std::unique_lock<std::mutex> lock(_mutex);

		_stopRequested = true;

		if (immediate)
		{
			// Clear function queue
			func_queue_type emptyQueue;
			_taskQueue.swap(emptyQueue);
		}
	}

	_cond.notify_one();
	_thread.join();
}

void TimerThread::_threadLoop()
{
	while (!(_stopRequested && _taskQueue.empty()))
	{
		std::function<void()> function;

		// Retrieve the next function to be run
		{
			std::unique_lock<std::mutex> lock(_mutex);

			if (_taskQueue.empty() && !_stopRequested)
			{
				_cond.wait(lock);
				continue;
			}
			else if (_taskQueue.top().scheduledTime > clock_type::now())
			{
				_cond.wait_until(lock, _taskQueue.top().scheduledTime);
				continue;
			}
			else
			{
				function = _taskQueue.top().function;
				_taskQueue.pop();
			}
		}

		// Run the function on its own task
		if (function)
		{
			pplx::task<void>(function).then([](pplx::task<void> result)
			{
				// We do not want to crash if the function throws, so handle any exception
				try
				{
					result.get();
				}
				catch (std::exception&)
				{
					// print
				}
			});
		}
		else if (!_stopRequested)
		{
			// User pushed an empty function.
			throw std::runtime_error("TimerThread: cannot run an empty function");
		}
	}
}

}