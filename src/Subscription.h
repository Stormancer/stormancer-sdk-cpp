#pragma once
#include "headers.h"
#include "Action.h"

namespace Stormancer
{
	class Subscription
	{
	public:
		Subscription();
		Subscription(Action<> unsubscribe);
		Subscription(Subscription& other);
		Subscription(Subscription&& other);
		Subscription& operator=(Subscription& other);
		virtual ~Subscription();

	public:
		bool subscribed() const;
		void unsubscribe();

	private:
		Action<> _unsubscribe;
		bool _subscribed = true;
	};
};
