#pragma once

#include "stormancer/BuildConfig.h"

#include <memory>
#include "stormancer/Tasks.h"
#include "stormancer/Compatibility/IStormancerTask.h"

namespace Stormancer
{

template<typename TRes>
class Task : public IStormancerTask<TRes>
{
public:

	using dispatcher_ptr = std::shared_ptr<pplx::scheduler_interface>;

	static Task_ptr<TRes> create(pplx::task<TRes>&& task, ILogger_ptr logger, dispatcher_ptr scheduler)
	{
		return std::make_shared<Task<TRes> >(std::move(task), logger, scheduler);
	}

	Task(pplx::task<TRes>&& task, ILogger_ptr logger, dispatcher_ptr scheduler) :
		_logger(logger),
		_dispatcher(scheduler)
	{
		_task = task.then([](pplx::task<TRes> taskRes)
		{
			StormancerResult<TRes> res;
			try
			{
				setResult(taskRes, res);
			}
			catch (std::exception& e)
			{
				res.SetError(e.what());
			}
#ifdef __cplusplus_winrt
			catch (Platform::Exception^ e)
			{
				res.SetError(Stormancer::wstring_to_utf8(L"HRESULT: " + std::to_wstring(e->HResult) + L", Message: " + e->Message->Data()).c_str());
			}
#endif

			return pplx::task_from_result(res);
		});
	}

	~Task()
	{
		// Task is kept alive until wrapped _task completes.
		// Crash on unhandled task error.
		if (!_resultWasHandled)
		{
			std::shared_ptr<ILogger> logger = _logger;
			_task.then([=](pplx::task<StormancerResult<TRes> > taskRes)
			{
				StormancerResult<TRes> res;
				try
				{
					res = taskRes.get();
				}
				catch (std::exception& e)
				{
					res.SetError(e.what());
				}

				if (!res.Success())
				{
					logger->log(LogLevel::Fatal, "StormancerTask", "Unhandled task error", res.Reason());
					assert(false);
				}
			});
		}
	}

	Task(const Task<TRes>& other) = delete;

	void Then(std::function<void(const StormancerResult<TRes>)> continuation) override
	{
		_resultWasHandled = true;

		// Schedule on the specified dispatcher, if any
		pplx::task_options opts = (_dispatcher) ? pplx::task_options(_dispatcher) : pplx::task_options();

		_task.then([=](pplx::task<StormancerResult<TRes> > taskRes)
		{
			StormancerResult<TRes> res;
			try
			{
				res = taskRes.get();
			}
			catch (std::exception& e)
			{
				// We are not supposed to get there...
				res.SetError(e.what());
			}

			continuation(res);
		}, opts);
	}

	bool IsDone() const override
	{
		return _task.is_done();
	}

	const StormancerResult<TRes> Get() override
	{
		_resultWasHandled = true;
		StormancerResult<TRes> res;
		try
		{
			res = _task.get();
		}
		catch (std::exception& e)
		{
			// We are not supposed to get there...
			res.SetError(e.what());
		}

		return res;
	}

private:

	static void setResult(const pplx::task<TRes>& task, StormancerResult<TRes>& res)
	{
		res.Set(task.get());
	}

private:
	//pplx::cancellation_token_source _cts;
	pplx::task<StormancerResult<TRes> > _task;
	bool _resultWasHandled = false;
	Stormancer::ILogger_ptr _logger;
	dispatcher_ptr _dispatcher;
};

template<>
void Task<void>::setResult(const pplx::task<void>& task, StormancerResult<void>& res)
{
	task.get();
	res.Set();
}

}