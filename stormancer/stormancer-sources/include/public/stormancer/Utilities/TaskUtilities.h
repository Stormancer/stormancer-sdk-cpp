#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Tasks.h"
#include "stormancer/TimerThread.h"

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

	template< class Rep, class Period >
	pplx::cancellation_token timeout(std::chrono::duration<Rep, Period> duration)
	{
		pplx::cancellation_token_source cts;

		TimerThread::getInstance().schedule([cts]()
		{
			cts.cancel();
		}, TimerThread::clock_type::now() + duration);

		return cts.get_token();
	}

	pplx::cancellation_token_source create_linked_source(pplx::cancellation_token token1, pplx::cancellation_token token2);
	
	pplx::cancellation_token_source create_linked_source(pplx::cancellation_token token1, pplx::cancellation_token token2, pplx::cancellation_token token3);

	pplx::cancellation_token create_linked_shutdown_token(pplx::cancellation_token token);

	// Cancels the provided task after the specifed delay, if the task
	// did not complete.
	template<typename T>
	pplx::task<T> cancel_after_timeout(pplx::task<T> t, unsigned int timeout)
	{
		// Create a task that returns true after the specified task completes.
		pplx::task<void> completedTask = t.then([](pplx::task<T>) {});

		// Create a task that returns false after the specified timeout.
		pplx::task<void> timeoutTask = taskDelay(std::chrono::milliseconds(timeout));

		// Create a continuation task that cancels the overall task 
		// if the timeout task finishes first.
		return (timeoutTask || completedTask).then([t]()
		{
			if (!t.is_done())
			{
				t.then([](pplx::task<T> innerT)
				{
					// We observe the timeouted task to prevent unobserved exceptions
					try
					{
						innerT.get();
					}
					catch (...) {}
				});

				pplx::cancel_current_task();
			}

			return t;
		});
	}

	// Cancels the provided task after the specifed delay, if the task
	// did not complete.
	template<typename T>
	pplx::task<T> cancel_after_timeout(pplx::task<T> t, pplx::cancellation_token_source cts, unsigned int timeout)
	{
		return cancel_after_timeout(t, timeout)
			.then([t, cts](pplx::task<T> innerT)
		{
			if (!t.is_done())
			{
				// Set the cancellation token. The task that is passed as the
				// t parameter should respond to the cancellation and stop
				// as soon as it can.
				cts.cancel();
			}

			return innerT;
		});
	}
}




namespace pplx

{
	pplx::cancellation_token operator||(pplx::cancellation_token token1, pplx::cancellation_token token2);
}
