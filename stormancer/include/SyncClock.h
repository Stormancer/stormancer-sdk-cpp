#pragma once

#include "headers.h"
#include "DependencyResolver.h"
#include "IConnection.h"
#include "Watch.h"

namespace Stormancer
{
	class SyncClock : public std::enable_shared_from_this<SyncClock>
	{
	public:

		SyncClock(DependencyResolver* resolver, int interval = 1000);
		~SyncClock();
		void start(std::weak_ptr<IConnection> connection, pplx::cancellation_token ct);
		int lastPing();
		bool isRunning();
		int64 clock();

	private:

		void stop();
		void syncClockImplAsync();

		struct ClockValue
		{
			double latency;
			double offset;
		};

		pplx::cancellation_token _cancellationToken = pplx::cancellation_token::none();
		std::weak_ptr<IConnection> _remoteConnection;
		DependencyResolver* _dependencyResolver;
		std::mutex _mutex;
		std::deque<ClockValue> _clockValues;
		Watch _watch;
		bool _isRunning = false;
		bool _lastPingFinished = true;
		unsigned int _maxValues = 24;
		int _lastPing = 0;
		int _interval = 1000;
		int _intervalAtStart = 100;
		double _medianLatency = 0;
		double _standardDeviationLatency = 0;
		double _offset = 0;
	};
};