#include "stormancer/stdafx.h"
#include "stormancer/SyncClock.h"
#include "stormancer/IScheduler.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/Helpers.h"
#include <numeric>

namespace Stormancer
{
	SyncClock::SyncClock(std::weak_ptr<DependencyResolver> dependencyResolver, int interval)
		: _dependencyResolver(dependencyResolver)
		, _interval(interval)
		, _logger(dependencyResolver.lock()->resolve<ILogger>())
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
		auto logger = _dependencyResolver.lock()->resolve<ILogger>();
		logger->log(LogLevel::Trace, "synchronizedClock", "Starting SyncClock...");

		if (!compareExchange(_mutex, _isRunning, false, true))
		{
			return;
		}

		auto connection = connectionPtr.lock();
		if (!connection)
		{
			throw std::runtime_error("The connection pointer is invalid");
		}
		_remoteConnection = connection;

		auto  scheduler = _dependencyResolver.lock()->resolve<IScheduler>();
		if (scheduler)
		{
			_cancellationToken = ct;

			auto cts = pplx::cancellation_token_source::create_linked_source(_cancellationToken);
			auto wSyncClock = STRM_WEAK_FROM_THIS();

			scheduler->schedulePeriodic(_interval, [wSyncClock, cts]()
			{
				auto syncClock = LockOrThrow(wSyncClock);

				std::lock_guard<std::mutex> lg(syncClock->_mutex);
				syncClock->syncClockImplAsync();
			}, _cancellationToken);

			// Do multiple pings at start
			scheduler->schedulePeriodic(_intervalAtStart, [wSyncClock, cts]
			{
				auto syncClock = LockOrThrow(wSyncClock);

				bool shouldCallSyncClockImpl = false;
				{
					std::lock_guard<std::mutex> lg(syncClock->_mutex);
					if (syncClock->_clockValues.size() >= syncClock->_maxValues)
					{
						cts.cancel();
					}
					else
					{
						shouldCallSyncClockImpl = true;
					}
				}
				if (shouldCallSyncClockImpl)
				{
					syncClock->syncClockImplAsync();
				}
			}, _cancellationToken);

			_cancellationToken.register_callback([wSyncClock]
			{
				auto syncClock = LockOrThrow(wSyncClock);
				syncClock->stop();
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
		auto logger = _logger;
		logger->log(LogLevel::Trace, "synchronizedClock", "Stopping SyncClock...");

		if (!compareExchange(_mutex, _isRunning, true, false))
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
		if (!_isRunning || !_lastPingFinished)
		{
			return;
		}

		try
		{
			_lastPingFinished = false;
			uint64 timeStart = _watch.getElapsedTime();
			auto requestProcessor = _dependencyResolver.lock()->resolve<RequestProcessor>();
			auto remoteConnection = _remoteConnection.lock();
			if (!remoteConnection)
			{
				throw std::runtime_error("Remote connection pointer is invalid.");
			}

			// Keep an active reference to the logger, in case the task is cancelled we cannot rely on this being valid
			auto logger = _dependencyResolver.lock()->resolve<ILogger>();
			auto cancellationToken = _cancellationToken;
			auto wSyncClock = STRM_WEAK_FROM_THIS();
			requestProcessor->sendSystemRequest(remoteConnection.get(), (byte)SystemRequestIDTypes::ID_PING, [&timeStart](obytestream* bs) {
				(*bs) << timeStart;
			}, PacketPriority::IMMEDIATE_PRIORITY, _cancellationToken)
				.then([wSyncClock, timeStart](Packet_ptr packet)
			{
				auto syncClock = LockOrThrow(wSyncClock);

				uint64 timeEnd = (uint64)syncClock->_watch.getElapsedTime();

				std::lock_guard<std::mutex> lock(syncClock->_mutex);

				syncClock->_lastPingFinished = true;

				uint64 timeServer;
				*packet->stream >> timeServer;

				uint16 ping = (uint16)(timeEnd - timeStart);
				syncClock->_lastPing = ping;
				double latency = ping / 2.0;

				double offset = timeServer - timeEnd + latency;

				syncClock->_clockValues.push_back(ClockValue{ latency, offset });
				if (syncClock->_clockValues.size() > syncClock->_maxValues)
				{
					syncClock->_clockValues.pop_front();
				}
				auto len = syncClock->_clockValues.size();

				std::vector<double> latencies(len);
				for (std::size_t i = 0; i < len; i++)
				{
					latencies[i] = syncClock->_clockValues[i].latency;
				}
				std::sort(latencies.begin(), latencies.end());
				syncClock->_medianLatency = latencies[len / 2];
				double pingAvg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / len;
				double varianceLatency = 0;
				for (auto v : latencies)
				{
					auto tmp = v - pingAvg;
					varianceLatency += (tmp * tmp);
				}
				varianceLatency /= len;
				syncClock->_standardDeviationLatency = std::sqrt(varianceLatency);

				double offsetsAvg = 0;
				uint32 lenOffsets = 0;
				double latencyMax = syncClock->_medianLatency + syncClock->_standardDeviationLatency;
				for (auto v : syncClock->_clockValues)
				{
					if (v.latency < latencyMax)
					{
						offsetsAvg += v.offset;
						lenOffsets++;
					}
				}
				syncClock->_offset = offsetsAvg / lenOffsets;
			}, _cancellationToken)
				.then([cancellationToken, logger](pplx::task<void> t)
			{
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					if (!cancellationToken.is_canceled())
					{
						logger->log(LogLevel::Warn, "syncClock", "Failed to ping the server", ex.what());
					}
				}
			});
		}
		catch (const std::exception& ex)
		{
			auto logger = _dependencyResolver.lock()->resolve<ILogger>();
			logger->log(LogLevel::Error, "Client::syncClockImpl", "Failed to ping server.", ex.what());
			throw std::runtime_error((std::string() + ex.what() + "\nFailed to ping server.").c_str());
		}
	}
}
