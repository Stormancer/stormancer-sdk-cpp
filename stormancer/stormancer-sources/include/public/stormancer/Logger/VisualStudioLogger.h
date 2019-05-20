#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Logger/ILogger.h"
#include <mutex>

namespace Stormancer
{
	class VisualStudioLogger : public Stormancer::ILogger
	{
	public:
		VisualStudioLogger(Stormancer::LogLevel maximalLogLevel = Stormancer::LogLevel::Trace);
		virtual ~VisualStudioLogger();

	public:
		virtual void log(const std::string& message) override;
		virtual void log(Stormancer::LogLevel level, const std::string& category, const std::string& message, const std::string& data = "") override;
		virtual void log(const std::exception& e) override;

	private:
		std::mutex _mutex;
		Stormancer::LogLevel _maximalLogLevel = Stormancer::LogLevel::Trace;
	};
}
