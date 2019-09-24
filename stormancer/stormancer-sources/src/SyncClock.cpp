#include "stormancer/stdafx.h"
#include "stormancer/SyncClock.h"
#include "stormancer/IScheduler.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/Helpers.h"
#include <numeric>
#include <cmath>

namespace Stormancer
{
	SyncClock::SyncClock(const DependencyScope& scope, int interval)
		: _dependencyResolver(scope.beginLifetimeScope())
		, _interval(interval)
		, _logger(scope.resolve<ILogger>())
	{
		_watch.reset();
	}

	SyncClock::~SyncClock()
	{
		if (_isRunning)
		{
			stop();
		}
	}

	void SyncClock::start(std::weak_ptr<IConnection> connectionPtr, pplx::cancellation_token ct)
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		auto logger = _dependencyResolver.resolve<ILogger>();
		logger->log(LogLevel::Trace, "synchronizedClock", "Starting SyncClock...");

		if (!compareExchange(_isRunning, false, true))
		{
			return;
		}

		auto connection = connectionPtr.lock();
		if (!connection)
		{
			throw std::runtime_error("The connection pointer is invalid");
		}
		_remoteConnection = connection;

		auto scheduler = _dependencyResolver.resolve<IScheduler>();
		if (scheduler)
		{
			auto cts = pplx::cancellation_token_source::create_linked_source(ct);
			_cancellationToken = cts.get_token();

			auto wSyncClock = STORM_WEAK_FROM_THIS();

			scheduler->schedulePeriodic(_interval, [wSyncClock, cts]()
			{
				auto syncClock = wSyncClock.lock();
				if (!syncClock)
				{
					cts.cancel();
					return;
				}

				syncClock->syncClockImplAsync();
			}, _cancellationToken);

			// Do multiple pings at start
			scheduler->schedulePeriodic(_intervalAtStart, [wSyncClock, cts]
			{
				auto syncClock = wSyncClock.lock();
				if (!syncClock)
				{
					cts.cancel();
					return;
				}

				std::lock_guard<std::recursive_mutex> lg(syncClock->_mutex);
				if (syncClock->_clockValues.size() >= syncClock->_maxValues)
				{
					cts.cancel();
				}
				else
				{
					syncClock->syncClockImplAsync();
				}
			}, _cancellationToken);

			_cancellationToken.register_callback([wSyncClock]
			{
				if (auto syncClock = wSyncClock.lock())
				{
					syncClock->stop();
				}
			});

			logger->log(LogLevel::Trace, "synchronizedClock", "SyncClock started");
		}
		else
		{
			logger->log(LogLevel::Warn, "synchronizedClock", "Missing scheduler");
		}
	}

	void SyncClock::stop()
	{
		std::lock_guard<std::recursive_mutex> lg(_mutex);

		auto logger = _logger;
		logger->log(LogLevel::Trace, "synchronizedClock", "Stopping SyncClock...");

		if (!compareExchange(_isRunning, true, false))
		{
			throw std::runtime_error("SyncClock is not started");
		}

		logger->log(LogLevel::Trace, "synchronizedClock", "SyncClock stopped");
	}

	int SyncClock::lastPing()
	{
		return _lastPing;
	}

	bool SyncClock::isRunning()
	{
		return _isRunning;
	}

	int64 SyncClock::clock()
	{
		return (int64)(_watch.getElapsedTime() + _offset);
	}

	void SyncClock::syncClockImplAsync()
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);

		if (!_isRunning || !_lastPingFinished)
		{
			return;
		}

		_lastPingFinished = false;
		_watch.reset();
		uint64 timeStart = _watch.getElapsedTime();
		auto requestProcessor = _dependencyResolver.resolve<RequestProcessor>();
		auto remoteConnection = _remoteConnection.lock();
		if (!remoteConnection)
		{
			throw std::runtime_error("Remote connection pointer is invalid.");
		}

		auto logger = _dependencyResolver.resolve<ILogger>();
		auto wSyncClock = STORM_WEAK_FROM_THIS();
		requestProcessor->sendSystemRequest(remoteConnection.get(), (byte)SystemRequestIDTypes::ID_PING, [&timeStart](obytestream& bs)
		{
			bs << timeStart;
		}, PacketPriority::IMMEDIATE_PRIORITY, _cancellationToken)
			.then([wSyncClock, timeStart](Packet_ptr packet)
		{
			if (auto syncClock = wSyncClock.lock())
			{
				uint64 timeServer;
				packet->stream >> timeServer;
				syncClock->processResult(timeStart, timeServer);
			}
		})
			.then([logger](pplx::task<void> t)
		{
			try
			{
				t.wait(); // wait because we want to ignore pplx::task_canceled
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Warn, "syncClock", "Failed to ping the server", ex.what());
			}
		});
	}

	void SyncClock::processResult(uint64 timeStart, uint64 timeServer)
	{
		uint64 timeEnd = (uint64)_watch.getElapsedTime();

		std::lock_guard<std::recursive_mutex> lock(_mutex);

		_lastPingFinished = true;

		uint16 ping = (uint16)(timeEnd - timeStart);
		_lastPing = ping;
		double latency = ping / 2.0;

		double offset = timeServer - timeEnd + latency;

		_clockValues.push_back(ClockValue{ latency, offset });
		if (_clockValues.size() > _maxValues)
		{
			_clockValues.pop_front();
		}
		auto len = _clockValues.size();

		std::vector<double> latencies(len);
		for (std::size_t i = 0; i < len; i++)
		{
			latencies[i] = _clockValues[i].latency;
		}
		std::sort(latencies.begin(), latencies.end());
		_medianLatency = latencies[len / 2];
		double pingAvg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / len;
		double varianceLatency = 0;
		for (auto v : latencies)
		{
			auto tmp = v - pingAvg;
			varianceLatency += (tmp * tmp);
		}
		varianceLatency /= len;
		_standardDeviationLatency = std::sqrt(varianceLatency);

		double offsetsAvg = 0;
		uint32 lenOffsets = 0;
		double latencyMax = _medianLatency + _standardDeviationLatency;
		for (auto v : _clockValues)
		{
			if (v.latency < latencyMax)
			{
				offsetsAvg += v.offset;
				lenOffsets++;
			}
		}
		_offset = offsetsAvg / lenOffsets;
	}
}
