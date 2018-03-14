#include "StormancerPluginPrivatePCH.h"
#include "StormancerLogger.h"

namespace Stormancer
{
	
	void FStormancerLogger::log(const std::string & message)
	{
		log(LogLevel::Info, "Stormancer", message);
	}

	void FStormancerLogger::log(LogLevel level, const std::string & category, const std::string & message, const std::string & data)
	{
		FString text = FString::Printf(TEXT("[%s] %s"), UTF8_TO_TCHAR(category.c_str()), UTF8_TO_TCHAR(message.c_str()));
		if (!data.empty())
		{
			text.Append(FString::Printf(TEXT(" : %s"), UTF8_TO_TCHAR(data.c_str())));
		}

		switch (level)
		{
		case LogLevel::Debug:
		case LogLevel::Info:
		case LogLevel::Trace:
			UE_LOG(StormancerLog, Log, TEXT("%s"), *text);
			break;
		case LogLevel::Warn:
			UE_LOG(StormancerLog, Warning, TEXT("%s"), *text);
			break;
		case LogLevel::Error:
			UE_LOG(StormancerLog, Error, TEXT("%s"), *text);
			break;
		case LogLevel::Fatal:
			UE_LOG(StormancerLog, Fatal, TEXT("%s"), *text);
			break;
		}
	}
	
	void FStormancerLogger::log(const std::exception& e)
	{
		log(LogLevel::Error, "Stormancer", "Caught exception", e.what());
	}

} // namespace Stormancer