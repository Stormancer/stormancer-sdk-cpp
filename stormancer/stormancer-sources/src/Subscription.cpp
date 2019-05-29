#pragma once

#include "stormancer/stdafx.h"
#include "stormancer/Subscription.h"

namespace Stormancer
{
	Subscription_impl::Subscription_impl(std::function<void(void)> callback)
		: _callback(callback)
	{
	}

	Subscription_impl::~Subscription_impl()
	{
		unsubscribe();
	}

	void Subscription_impl::unsubscribe()
	{
		if (_callback)
		{
			_callback();
			_callback = nullptr;
		}
	}
}
