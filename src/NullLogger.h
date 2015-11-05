#pragma once
#include "headers.h"
#include "ILogger.h"

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
		void log(const char* message);
		
		/// Empty implementation.
		void log(LogLevel level, const char* category, const char* message, const char* data);
		
		/// Empty implementation.
		void log(const std::exception& e);
	};
};
