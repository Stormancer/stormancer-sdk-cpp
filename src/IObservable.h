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

		virtual ISubscription* subscribe(std::function<void(T)>) = 0;

		virtual void destroy() = 0;
	};
}
