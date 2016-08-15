#pragma once
#include "headers.h"
#include "ITask.h"

namespace Stormancer
{
	template<typename T>
	class Task : public ITask<T>
	{
	public:
		Task(pplx::task<T> t)
			: _task(t)
		{
		}

		T get() const
		{
			return _task.get();
		}

		bool is_done() const
		{
			return _task.is_done();
		}

		template<typename T2 = void>
		ITask<T2>* then(std::function<T2(T)> function)
		{
			return new Task(_task.then(function));
		}

		template<typename T2 = void>
		ITask<T2>* fail(std::function<T2(const char* errorMessage)> function)
		{
			return new Task(_task.then([function](pplx::task<T> t) {
				try
				{
					t.wait();
				}
				catch (const std::exception& ex)
				{
					function(ex.what());
				}
			}));
		}

		template<typename T2 = void>
		ITask<T2>* always(std::function<T2()> function)
		{
			return new Task(_task.then([](pplx::task<T> t) {
				try
				{
					t.wait();
				}
				catch (...)
				{
				}

				function();
			}));
		}

		void wait() const
		{
			_task.wait();
		}

	private:
		pplx::task<T> _task;
	};
}
