#pragma once
#include "headers.h"

namespace Stormancer
{
	template<typename T>
	class ITask
	{
	public:
		ITask()
		{
			pplx::task<int> t;
			t.get();
			t.is_done();
			t.then([](int) {});
		}

		virtual T get() const = 0;

		virtual bool is_done() const = 0;

		template<typename T2 = void>
		virtual ITask<T2>* then(std::function<T2(T)> function) = 0;
		
		template<typename T2 = void>
		virtual ITask<T2>* fail(std::function<T2(const char* errorMessage)> function) = 0;
		
		template<typename T2 = void>
		virtual ITask<T2>* always(std::function<T2()> function) = 0;

		void wait() const = 0;

		virtual void destroy() = 0;
	};
}
