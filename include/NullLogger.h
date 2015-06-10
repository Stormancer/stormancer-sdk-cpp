#pragma once
#include "headers.h"
#include "ILogger.h"

namespace Stormancer
{
	/// Inactive logger (Used by default).
	class NullLogger : public ILogger
	{
	public:
		NullLogger();
		~NullLogger();

	public:
		void log(wstring message);
		void log(LogLevel level, wstring category, wstring message, wstring data);
		void log(const std::exception& e);
	};
};
