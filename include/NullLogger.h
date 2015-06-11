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
		void log(wstring message);
		
		/// Empty implementation.
		void log(LogLevel level, wstring category, wstring message, wstring data);
		
		/// Empty implementation.
		void log(const std::exception& e);
	};
};
