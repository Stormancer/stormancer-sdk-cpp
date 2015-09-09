#include "stormancer.h"

namespace Stormancer
{
	Subscription::Subscription()
	{
	}

	Subscription::Subscription(Action<> unsubscribe)
		: _unsubscribe(unsubscribe)
	{
	}

	Subscription::Subscription(Subscription& other)
		: _unsubscribe(other._unsubscribe),
		_subscribed(other._subscribed)
	{
	}

	Subscription::Subscription(Subscription&& other)
		: _unsubscribe(other._unsubscribe),
		_subscribed(other._subscribed)
	{
	}

	Subscription& Subscription::operator=(Subscription& other)
	{
		_unsubscribe = std::move(other._unsubscribe);
		_subscribed = other._subscribed;
		return *this;
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
		_unsubscribe();
		_subscribed = false;
	}
};
