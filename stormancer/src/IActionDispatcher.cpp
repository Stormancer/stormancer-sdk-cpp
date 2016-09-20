#include "IActionDispatcher.h"

namespace Stormancer
{
	bool SameThreadActionDispatcher::isRunning()
	{
		return _isRunning;
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

	void MainThreadActionDispatcher::update(uint32 maxDuration)
	{
		if (!_isRunning)
		{
			return;
		}

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
		}
	}
}