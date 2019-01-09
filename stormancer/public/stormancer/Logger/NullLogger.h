#pragma once

#include "stormancer/headers.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	/// Inactive logger (Used by default).
	class NullLogger : public ILogger
	{
	public:
		
		/// Constructor.
		NullLogger();
		
		/// Destructor.
		~NullLogger();

	public:
	
		/// Empty implementation.
		void log(const std::string& message);
		
		/// Empty implementation.
		void log(LogLevel level, const std::string& category, const std::string& message, const std::string& data = "");
		
		/// Empty implementation.
		void log(const std::exception& ex);
	};
};
