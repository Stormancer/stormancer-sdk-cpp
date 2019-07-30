#include "stormancer/stdafx.h"
#include "stormancer/Watch.h"

namespace Stormancer
{
	Watch::Watch()
	{
		reset();
	}

	Watch::~Watch()
	{
	}

	void Watch::reset()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_startTime = std::chrono::steady_clock::now();
	}

	int64 Watch::getElapsedTime()
	{
		std::chrono::steady_clock::time_point startTime;
		int64 baseTime;
		{
			std::lock_guard<std::mutex> lock(_mutex);
			startTime = _startTime;
			baseTime = _baseTime;
		}
		auto now = std::chrono::steady_clock::now();
		auto dif = now - startTime;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dif);
		return ms.count() + baseTime;
	}

	void Watch::setBaseTime(int64 baseTime)
	{
		reset();
		std::lock_guard<std::mutex> lock(_mutex);
		_baseTime = baseTime;
	}
}
