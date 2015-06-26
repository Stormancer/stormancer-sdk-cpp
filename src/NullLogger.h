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
		void log(std::string message);
		
		/// Empty implementation.
		void log(LogLevel level, std::string category, std::string message, std::string data);
		
		/// Empty implementation.
		void log(const std::exception& e);
	};
};
