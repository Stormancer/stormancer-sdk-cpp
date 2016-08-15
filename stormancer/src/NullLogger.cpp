#include "stormancer.h"

namespace Stormancer
{
	NullLogger::NullLogger()
	{
	}

	NullLogger::~NullLogger()
	{
	}

	void NullLogger::log(const char* message)
	{
	}

	void NullLogger::log(LogLevel level, const char* category, const char* message, const char* data)
	{
	}

	void NullLogger::log(const std::exception& e)
	{
	}
};
