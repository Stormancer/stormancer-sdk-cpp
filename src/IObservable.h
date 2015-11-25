#pragma once
#include "Action.h"

namespace Stormancer
{
	template<typename T>
	class IObservable
	{
	public:
		virtual ~IObservable()
		{
		}

		virtual ISubscription* subscribe(std::function<void(T)> onNext) = 0;
		virtual ISubscription* subscribe(std::function<void(T)> onNext, std::function<void(std::exception_ptr)> onError) = 0;
		virtual ISubscription* subscribe(std::function<void(T)> onNext, std::function<void(std::exception_ptr)> onError, std::function<void()> onComplete) = 0;
		virtual ISubscription* subscribe(std::function<void(T)> onNext, std::function<void()> onComplete) = 0;

		virtual void destroy() = 0;
	};
}
