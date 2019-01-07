#pragma once

#include "stormancer/BuildConfig.h"

#include <functional>
#include <memory>
#include "stormancer/Compatibility/StormancerResult.h"

template<typename TRes>
class IStormancerTask
{
public:

	/// Schedule the execution of \p continuation after the task has run.
	/// The argument to \p continuation will contain the task's result if its execution was successful,
	/// or an error string if it wasn't.
	virtual void Then(std::function<void(const StormancerResult<TRes>)> Continuation) = 0;

	/// Check whether the task has finished running
	virtual bool IsDone() const = 0;

	/// Retrieve the task's result. Calling this method is allowed only when IsDone returns \c true.
	virtual const StormancerResult<TRes> Get() = 0;
};

template<typename TRes>
using Task_ptr = std::shared_ptr<IStormancerTask<TRes> >;