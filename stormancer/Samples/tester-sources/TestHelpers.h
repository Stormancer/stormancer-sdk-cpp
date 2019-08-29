#pragma once
#include <memory>
#include "stormancer/Tasks.h"
#include "stormancer/Utilities/TaskUtilities.h"

namespace TestHelpers
{
	template<typename T>
	class SelfObservingTask
	{
	public:
		SelfObservingTask(pplx::task<T> task) : _impl(std::make_shared<impl>(task)) {}

		pplx::task<T>* operator->()
		{
			return &(_impl->task);
		}

		pplx::task<T> operator*() const
		{
			return _impl->task;
		}

	private:
		struct impl
		{
			pplx::task<T> task;

			impl(pplx::task<T> task) : task(task) {}

			~impl()
			{
				task.then([](pplx::task<T> t)
				{
					try
					{
						t.get();
					}
					catch (...) {}
				});
			}
		};

		std::shared_ptr<impl> _impl;
	};

	void failAfterTimeout(pplx::task_completion_event<void> tce, std::string msg, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

	void failAfterTimeout(pplx::task_completion_event<void> tce, std::function<std::string()> msgBuilder, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

	template<typename T>
	pplx::task<T> taskFailAfterTimeout(pplx::task<T> task, std::string msg, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
	{
		return Stormancer::cancel_after_timeout(task, static_cast<unsigned int>(timeout.count()))
			.then([msg](pplx::task<T> t)
		{
			auto status = t.wait();
			if (status == pplx::canceled)
			{
				throw std::runtime_error(msg);
			}
			return t;
		});
	}

}