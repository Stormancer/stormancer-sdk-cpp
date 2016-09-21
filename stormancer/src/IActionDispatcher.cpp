#include "IActionDispatcher.h"

namespace Stormancer
{
	bool SameThreadActionDispatcher::isRunning()
	{
		return _isRunning;
	}

	void SameThreadActionDispatcher::schedule(pplx::TaskProc_t task, void * param)
	{
		task(param);
	}


	void SameThreadActionDispatcher::start()
	{
		_isRunning = true;
	}

	void SameThreadActionDispatcher::stop()
	{
		_isRunning = false;
	}

	void SameThreadActionDispatcher::post(const std::function<void(void)> action)
	{
		action();
	}


	void MainThreadActionDispatcher::start()
	{
		_isRunning = true;
		_stopRequested = false;
		while (_actions.size())
		{
			_actions.pop();
		}
	}

	//Stop will be effective only on the next execution of "run"
	void MainThreadActionDispatcher::stop()
	{
		_stopRequested = true;
	}

	bool MainThreadActionDispatcher::isRunning()
	{
		return _isRunning;
	}

	void MainThreadActionDispatcher::post(const std::function<void(void)> action)
	{
		if (_isRunning)
		{
			_actions.push(action);
		}
	}

	void MainThreadActionDispatcher::update(std::chrono::milliseconds maxDuration)
	{
		if (!_isRunning)
		{
			return;
		}
		auto start  = std::chrono::system_clock::now();
		while (_actions.size() || _stopRequested)
		{

			if (_stopRequested)
			{
				while (_actions.size())
				{
					_actions.pop();
				}
				_isRunning = false;
				return;
			}

			auto action = _actions.front();
			action();
			_actions.pop();

			if (std::chrono::system_clock::now() - start > maxDuration)
			{
				return;
			}
		}
	}
	void MainThreadActionDispatcher::schedule(pplx::TaskProc_t task, void * param)
	{
		post([task, param]() {task(param); });
	}
}