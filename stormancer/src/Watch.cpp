#include "stormancer.h"

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
		_startTime = std::chrono::high_resolution_clock::now();
	}

	int64 Watch::getElapsedTime()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto dif = now - _startTime;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dif);
		return ms.count() + _baseTime;
	}

	void Watch::setBaseTime(int64 baseTime)
	{
		reset();
		_baseTime = baseTime;
	}
};
