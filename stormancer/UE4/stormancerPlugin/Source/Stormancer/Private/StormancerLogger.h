#pragma once
#include "stormancer/Logger/ILogger.h"
#include "IStormancerPlugin.h"

namespace Stormancer
{
	class FStormancerLogger : public ILogger
	{
	public:
		
		/// Logs a simple message
		void log(const std::string& message) override;

		/// Logs a full message
		void log(LogLevel level, const std::string& category, const std::string& message, const std::string& data = "") override;

		/// Logs an exception
		void log(const std::exception& e) override;
	};
} // namespace Stormancer