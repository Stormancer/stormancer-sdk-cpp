#include "stormancer/stdafx.h"
#include "stormancer/Logger/VisualStudioLogger.h"
#include <locale>

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
		log(Stormancer::LogLevel::Info, "", message, "");
	}

	void VisualStudioLogger::log(Stormancer::LogLevel level, const std::string& category, const std::string& message, const std::string& data)
	{
		if (level > _maximalLogLevel)
		{
			return;
		}
		std::string message2 = format(level, category, message, data) + "\n";

		{
			std::lock_guard<std::mutex> lg(_mutex);

			OutputDebugStringA(message2.c_str());
		}
	}

	void VisualStudioLogger::log(const std::exception& ex)
	{
		std::string msg = formatException(ex) + "\n";

		{
			std::lock_guard<std::mutex> lg(_mutex);

			OutputDebugStringA(msg.c_str());
		}
	}
}
