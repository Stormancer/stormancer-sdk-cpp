#include "stormancer.h"

namespace Stormancer
{
	NullLogger::NullLogger()
	{
	}

	NullLogger::~NullLogger()
	{
	}

	void NullLogger::log(const std::string message)
	{
	}

	void NullLogger::log(LogLevel level, const std::string category, const std::string message, const std::string data)
	{
	}

	void NullLogger::log(const std::exception& e)
	{
	}
};
