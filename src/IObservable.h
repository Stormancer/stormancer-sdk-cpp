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

		virtual void subscribe(std::function<void(T)>) = 0;

		virtual void destroy()
		{
			delete this;
		}

	private:
		rxcpp::observable<T> _observable;
	};
}
