#include "SyncClock.h"

Stormancer::SyncClock::SyncClock(DependencyResolver * resolver, int interval)
{
	this->_resolver = resolver;
	
	this->_synchronisedClockInterval = interval;
	this->_watch.reset();
}

void Stormancer::SyncClock::Start(IConnection* connection)
{
	if (_isRunning)
	{
		return;
	}
	_isRunning = true;
	_remoteConnection = connection;
	auto logger = _resolver->resolve<ILogger>();
	logger->log(LogLevel::Trace, "synchronizedClock", "Starting syncClock.", "");
	std::lock_guard<std::mutex> lg(_syncClockMutex);
	auto  scheduler = _resolver->resolve<IScheduler>();
	if (scheduler)
	{
		int interval = (int)(_clockValues.size() >= _maxClockValues ? _synchronisedClockInterval : _pingIntervalAtStart);
		_syncClockSubscription = scheduler->schedulePeriodic(interval, [this, interval]() {
			std::lock_guard<std::mutex> lg(_syncClockMutex);
			if (_syncClockSubscription.is_subscribed())
			{
				if (interval == _pingIntervalAtStart && _clockValues.size() >= _maxClockValues)
				{
					pplx::task<void>([this]() {
						this->Stop();
						this->Start(this->_remoteConnection);
					});
				}
				else
				{
					syncClockImpl();
				}
			}
		});
	}
	syncClockImpl();
	logger->log(LogLevel::Trace, "synchronizedClock", "SyncClock started", "");
}

void Stormancer::SyncClock::Stop()
{
	if (!_isRunning)
	{
		return;
	}

	auto logger = _resolver->resolve<ILogger>();
	ILogger::instance()->log(LogLevel::Trace, "synchronizedClock", "waiting to stop syncClock", "");
	std::lock_guard<std::mutex> lg(_syncClockMutex);
	if (_syncClockSubscription.is_subscribed())
	{
		_syncClockSubscription.unsubscribe();
	}
	logger->log(LogLevel::Trace, "synchronizedClock", "SyncClock stopped", "");
	_isRunning = false;
}

int Stormancer::SyncClock::LastPing()
{
	if (_isRunning)
	{
		return _lastPing;
	}
	else
	{
		throw std::runtime_error("Synchronized clock must be running to get ping values.");
	}
}

bool Stormancer::SyncClock::IsRunning()
{
	return _isRunning;
}

Stormancer::int64 Stormancer::SyncClock::Clock()
{
	return (int64)(_watch.getElapsedTime() + _offset);
}

pplx::task<void> Stormancer::SyncClock::syncClockImpl()
{
	
	if (!lastPingFinished)
	{
		return pplx::task_from_result();
	}

	try
	{
		lastPingFinished = false;

		uint64 timeStart = _watch.getElapsedTime();
		auto requestProcessor = _resolver->resolve<RequestProcessor>();
		return requestProcessor->sendSystemRequest(_remoteConnection, (byte)SystemRequestIDTypes::ID_PING, [&timeStart](bytestream* bs) {
			*bs << timeStart;
		}, PacketPriority::IMMEDIATE_PRIORITY).then([this, timeStart](pplx::task<Packet_ptr> t) {
			Packet_ptr packet;
			try
			{
				packet = t.get();
			}
			catch (const std::exception& ex)
			{
				auto logger = _resolver->resolve<ILogger>();
				logger->log(LogLevel::Warn, "syncClock", "Failed to ping the server", ex.what());
				return;
			}

			uint64 timeEnd = (uint16)this->_watch.getElapsedTime();

			lastPingFinished = true;

			uint64 timeServer;
			*packet->stream >> timeServer;

			uint16 ping = (uint16)(timeEnd - timeStart);
			_lastPing = ping;
			double latency = ping / 2.0;

			double offset = timeServer - timeEnd + latency;

			_clockValues.push_back(ClockValue{ latency, offset });
			if (_clockValues.size() > _maxClockValues)
			{
				_clockValues.pop_front();
			}
			auto len = _clockValues.size();

			std::vector<double> latencies(len);
			for (auto i = 0; i < len; i++)
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
		});
	}
	catch (const std::exception& ex)
	{
		auto logger = _resolver->resolve<ILogger>();
		logger->log(LogLevel::Error, "Client::syncClockImpl", "Failed to ping server.", ex.what());
		throw std::runtime_error(std::string() + ex.what() + "\nFailed to ping server.");
	}
}
