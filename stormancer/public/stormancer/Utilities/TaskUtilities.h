#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Tasks.h"
namespace Stormancer
{
	pplx::task<void> taskIf(bool condition, std::function<pplx::task<void>()> action);

	pplx::task<void> taskDelay(std::chrono::milliseconds timeOffset, pplx::cancellation_token ct = pplx::cancellation_token::none());

	template<typename T, typename... U>
	pplx::task<T> invokeWrapping(std::function<pplx::task<T>(U...)> func, U&... argument)
	{
		try
		{
			return func(argument...);
		}
		catch (const std::exception& ex)
		{
			return pplx::task_from_exception<T>(ex);
		}
	}
};