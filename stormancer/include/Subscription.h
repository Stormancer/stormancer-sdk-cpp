#pragma once
#include "headers.h"
#include "Action.h"
#include "ISubscription.h"

namespace Stormancer
{
	class Subscription : public ISubscription
	{
	public:
		Subscription();
		Subscription(std::function<void()> unsubscribe);
		Subscription(Action<> unsubscribe);
		virtual ~Subscription();

	public:
		STORMANCER_DLL_API bool subscribed() const;
		STORMANCER_DLL_API void unsubscribe();
		STORMANCER_DLL_API void destroy();

	private:
		Action<> _unsubscribe;
		bool _subscribed = true;
	};
};
