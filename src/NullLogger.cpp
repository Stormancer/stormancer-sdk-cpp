#include "stormancer.h"

namespace Stormancer
{
	NullLogger::NullLogger()
	{
	}

	NullLogger::~NullLogger()
	{
	}

	void NullLogger::log(std::string message)
	{
	}

	void NullLogger::log(LogLevel level, std::string category, std::string message, std::string data)
	{
	}

	void NullLogger::log(const std::exception& e)
	{
	}
};
