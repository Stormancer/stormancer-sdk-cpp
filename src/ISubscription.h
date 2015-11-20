#pragma once
#include "headers.h"
#include "Action.h"

namespace Stormancer
{
	class ISubscription
	{
	public:
		virtual bool subscribed() const = 0;
		virtual void unsubscribe() = 0;
		virtual void destroy() = 0;
	};
};
