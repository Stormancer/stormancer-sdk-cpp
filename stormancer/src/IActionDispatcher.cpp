#include "stormancer/stdafx.h"
#include "stormancer/IActionDispatcher.h"

namespace Stormancer
{
	bool SameThreadActionDispatcher::isRunning()
	{
		return _isRunning;
	}

	SameThreadActionDispatcher::~SameThreadActionDispatcher()
	{

	}

	void SameThreadActionDispatcher::schedule(pplx::TaskProc_t task, void* param)
	{
		task(param);
	}

	void SameThreadActionDispatcher::start()
	{
		_isRunning = true;
	}

	pplx::task<void> SameThreadActionDispatcher::stop()
	{
		_isRunning = false;
		return pplx::task_from_result();
	}

	void SameThreadActionDispatcher::post(const std::function<void(void)>& action)
	{
		if (_isRunning)
		{
			action();
		}
		else
		{
			throw std::logic_error("Dispatcher isn't running");
		}
	}



	void MainThreadActionDispatcher::start()
	{
		std::lock_guard<std::mutex> l(_mutex);

		_isRunning = true;
		_stopRequested = false;
		_actions = std::queue<std::function<void()>>();
	}

	//Stop will be effective only on the next execution of "run"
	pplx::task<void> MainThreadActionDispatcher::stop()
	{
		std::lock_guard<std::mutex> l(_mutex);

		_stopRequested = true;
		return pplx::create_task(_stopTce);
	}

	bool MainThreadActionDispatcher::isRunning()
	{
		return _isRunning;
	}

	MainThreadActionDispatcher::~MainThreadActionDispatcher()
	{

	}

	void MainThreadActionDispatcher::post(const std::function<void(void)>& action)
	{
		std::lock_guard<std::mutex> l(_mutex);

		if (_isRunning)
		{
			_actions.push(action);
		}
		else
		{
			throw std::logic_error("Dispatcher isn't running");
		}
	}

	void MainThreadActionDispatcher::update(std::chrono::milliseconds maxDuration)
	{
		if (!_isRunning)
		{
			return;
		}

		auto start = std::chrono::system_clock::now();
		bool tasksToRun = true;

		do
		{
			std::function<void()> action;
			{
				// Retrieve in an inner scope to avoid keeping the mutex when calling action()
				std::lock_guard<std::mutex> l(_mutex);

				if (!_actions.empty())
				{
					action = _actions.front();
					_actions.pop();
				}
				else
				{
					tasksToRun = false;
					if (_stopRequested)
					{
						// Wait for the queue to be empty before stopping
						_isRunning = false;
						_stopTce.set();
					}
				}
			}
			if (tasksToRun)
			{
				action();
			}

			if (std::chrono::system_clock::now() - start > maxDuration)
			{
				return;
			}
		} while (tasksToRun || _stopRequested);
	}

	void MainThreadActionDispatcher::schedule(pplx::TaskProc_t task, void* param)
	{
		post([=]() {
			task(param);
		});
	}

	IActionDispatcher::~IActionDispatcher()
	{

	}

}
