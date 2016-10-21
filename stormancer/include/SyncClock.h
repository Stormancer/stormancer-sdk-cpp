#pragma once
#include "stormancer.h"
namespace Stormancer
{
	class SyncClock
	{
	public:
		SyncClock(DependencyResolver* resolver, int interval);
		void Start(IConnection* connection);
		void Stop();

		int LastPing();
		bool IsRunning();

		int64 Clock();
	private:
		pplx::task<void> syncClockImpl();

	private:

		struct ClockValue
		{
			double latency;
			double offset;
		};

		IConnection* _remoteConnection;

		DependencyResolver* _resolver;
		std::mutex _syncClockMutex;
		std::deque<ClockValue> _clockValues;
		double _medianLatency = 0;
		double _standardDeviationLatency = 0;
		uint16 _maxClockValues = 24;

		int64 _synchronisedClockInterval = 5000;
		int64 _pingIntervalAtStart = 100;

		rxcpp::subscription _syncClockSubscription;
		bool lastPingFinished = true;
		bool _isRunning = false;
		Watch _watch;
		int _lastPing = 0;
		double _offset = 0;
	};
};