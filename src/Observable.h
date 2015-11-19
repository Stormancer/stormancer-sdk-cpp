#pragma once
#include "headers.h"
#include "IObservable.h"
#include "Action.h"

namespace Stormancer
{
	template<typename T>
	class Observable : public IObservable<T>
	{
	public:
		Observable(rxcpp::observable<T> observable)
			: _observable(observable)
		{
		}

		virtual ~Observable()
		{
		}

		void subscribe(std::function<void(T)> f)
		{
			_observable.subscribe(f);
		}
	};
};
