#pragma once

namespace TestHelpers
{
	template<typename T>
	class SelfObservingTask
	{
	public:
		SelfObservingTask(pplx::task<T> task) : task(task) {}
		~SelfObservingTask()
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

		pplx::task<T> task;

		pplx::task<T>* operator->()
		{
			return &task;
		}

		pplx::task<T> operator*() const
		{
			return task;
		}
	};
}