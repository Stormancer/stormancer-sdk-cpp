#include "stormancer/stdafx.h"
#include "stormancer/SyncClock.h"
#include "stormancer/IScheduler.h"
#include "stormancer/RequestProcessor.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/SafeCapture.h"

namespace Stormancer
{
	SyncClock::SyncClock(DependencyResolver* dependencyResolver, int interval)
		: _dependencyResolver(dependencyResolver)
		, _interval(interval)
		, _logger(dependencyResolver->resolve<ILogger>())
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
		auto logger = _dependencyResolver->resolve<ILogger>();
		logger->log(LogLevel::Trace, "synchronizedClock", "Starting SyncClock...");

		if (!compareExchange(_mutex, _isRunning, false, true))
		{
			throw std::runtime_error("SyncClock already started");
		}

		auto connection = connectionPtr.lock();
		if (!connection)
		{
			throw std::runtime_error("The connection pointer is invalid");
		}
		_remoteConnection = connection;

		auto  scheduler = _dependencyResolver->resolve<IScheduler>();
		if (scheduler)
		{
			_cancellationToken = ct;

			auto cts = pplx::cancellation_token_source::create_linked_source(_cancellationToken);
			scheduler->schedulePeriodic(_interval, STRM_SAFE_CAPTURE([this, cts]()
			{
				std::lock_guard<std::mutex> lg(_mutex);
				syncClockImplAsync();
			}), _cancellationToken);

			// Do multiple pings at start
			scheduler->schedulePeriodic(_intervalAtStart, STRM_SAFE_CAPTURE([this, cts]
			{
				bool shouldCallSyncClockImpl = false;
				{
					std::lock_guard<std::mutex> lg(_mutex);
					if (_clockValues.size() >= _maxValues)
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
					syncClockImplAsync();
				}
			}), _cancellationToken);

			_cancellationToken.register_callback(STRM_SAFE_CAPTURE([this]
			{
				stop();
			}));

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
			auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();
			auto remoteConnection = _remoteConnection.lock();
			if (!remoteConnection)
			{
				throw std::runtime_error("Remote connection pointer is invalid.");
			}

			// Keep an active reference to the logger, in case the task is cancelled we cannot rely on this being valid
			auto logger = _dependencyResolver->resolve<ILogger>();
			auto cancellationToken = _cancellationToken;
			requestProcessor->sendSystemRequest(remoteConnection.get(), (byte)SystemRequestIDTypes::ID_PING, [&timeStart](obytestream* bs) {
				(*bs) << timeStart;
			}, PacketPriority::IMMEDIATE_PRIORITY, _cancellationToken)
				.then(STRM_SAFE_CAPTURE([=](Packet_ptr packet)
			{
				uint64 timeEnd = (uint64)_watch.getElapsedTime();

				std::lock_guard<std::mutex> lock(_mutex);

				_lastPingFinished = true;

				uint64 timeServer;
				*packet->stream >> timeServer;

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
			}), _cancellationToken)
				.then([=](pplx::task<void> t)
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
			auto logger = _dependencyResolver->resolve<ILogger>();
			logger->log(LogLevel::Error, "Client::syncClockImpl", "Failed to ping server.", ex.what());
			throw std::runtime_error(std::string() + ex.what() + "\nFailed to ping server.");
		}
	}
}
