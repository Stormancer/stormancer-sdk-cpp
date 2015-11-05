#pragma once
#include "headers.h"

namespace Stormancer
{
	class Watch
	{
	public:
		Watch();
		~Watch();

	public:
		void reset();
		int64 getElapsedTime();
		void setBaseTime(int64 baseTime);

	private:
		std::chrono::steady_clock::time_point _startTime;
		int64 _baseTime = 0;
	};
};
