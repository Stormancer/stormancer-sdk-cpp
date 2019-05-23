#pragma once

#include "stormancer/stdafx.h"
#include "stormancer/Subscription.h"

namespace Stormancer
{
	Subscription_impl::Subscription_impl(std::function<void(void)> destroy)
		: _destroy(destroy)
	{
	}

	/// Call the destroy callback when deleted
	Subscription_impl::~Subscription_impl()
	{
		_destroy();
	}
}
