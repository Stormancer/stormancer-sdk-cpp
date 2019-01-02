#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/StormancerTypes.h"
#include <chrono>
#include <mutex>

namespace Stormancer
{
	class Watch
	{
	public:

#pragma region public_methods

		Watch();
		~Watch();
		void reset();
		int64 getElapsedTime();
		void setBaseTime(int64 baseTime);

#pragma endregion

	private:

#pragma region public_methods

		std::chrono::steady_clock::time_point _startTime;
		int64 _baseTime = 0;
		std::mutex _mutex;

#pragma endregion
	};
};
