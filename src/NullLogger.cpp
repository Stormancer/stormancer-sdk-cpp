#include "stormancer.h"

namespace Stormancer
{
	NullLogger::NullLogger()
	{
	}

	NullLogger::~NullLogger()
	{
	}

	void NullLogger::log(LogLevel level, wstring category, wstring message, wstring data)
	{
	}
};
