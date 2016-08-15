#include "stormancer.h"

namespace Stormancer
{
	Subscription::Subscription()
	{
	}

	Subscription::Subscription(std::function<void()> unsubscribe)
	{
		_unsubscribe += unsubscribe;
	}

	Subscription::Subscription(Action<> unsubscribe)
		: _unsubscribe(unsubscribe)
	{
	}

	Subscription::~Subscription()
	{
		unsubscribe();
	}

	bool Subscription::subscribed() const
	{
		return _subscribed;
	}

	void Subscription::unsubscribe()
	{
		if (_subscribed)
		{
			_unsubscribe();
			_subscribed = false;
		}
	}

	void Subscription::destroy()
	{
		delete this;
	}
};
