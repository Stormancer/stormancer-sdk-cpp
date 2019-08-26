#pragma once
#include <memory>
#include "stormancer/Tasks.h"

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
}