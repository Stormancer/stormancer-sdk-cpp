#include "stdafx.h"
#include "stormancer/Logger/VisualStudioLogger.h"
#include <locale>

namespace
{
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t> > wcharConverter;
}

namespace Stormancer
{
	VisualStudioLogger::VisualStudioLogger(Stormancer::LogLevel maximalLogLevel)
		: _maximalLogLevel(maximalLogLevel)
	{
	}

	VisualStudioLogger::~VisualStudioLogger()
	{
	}

	void VisualStudioLogger::log(const std::string& message)
	{
		log(Stormancer::LogLevel::Info, "None", message, "");
	}

	void VisualStudioLogger::log(Stormancer::LogLevel level, const std::string& category, const std::string& message, const std::string& data)
	{
		if (level > _maximalLogLevel)
		{
			return;
		}
		std::string message2 = format(level, category, message, data) + "\n";

		_mutex.lock();

		OutputDebugString(wcharConverter.from_bytes(message2).c_str());

		_mutex.unlock();
	}

	void VisualStudioLogger::log(const std::exception& e)
	{
		_mutex.lock();

		std::string msg(formatException(e));
		msg += "\n";

		OutputDebugString(wcharConverter.from_bytes(msg).c_str());
		_mutex.unlock();
	}
}
