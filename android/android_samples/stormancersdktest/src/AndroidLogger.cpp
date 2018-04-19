#include "AndroidLogger.h"
#include <android/log.h>

#define APPNAME "stormancersdktest"

AndroidLogger::AndroidLogger(Stormancer::LogLevel maximalLogLevel)
	: _maximalLogLevel(maximalLogLevel)
{
}

AndroidLogger::~AndroidLogger()
{
}

void AndroidLogger::log(std::string message)
{
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", message.c_str());
}

void AndroidLogger::log(Stormancer::LogLevel level, std::string category, std::string message, std::string data)
{
	if (level > _maximalLogLevel)
	{
		return;
	}

	std::string message2 = format(level, category, message, data);
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", message2.c_str());
}

void AndroidLogger::log(const std::exception& e)
{
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", formatException(e).c_str());
}
