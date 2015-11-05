#pragma once
#include "headers.h"
#include "Action.h"

namespace Stormancer
{
	class Subscription
	{
	public:
		STORMANCER_DLL_API Subscription();
		STORMANCER_DLL_API Subscription(Action<> unsubscribe);
		STORMANCER_DLL_API Subscription(Subscription& other);
		STORMANCER_DLL_API Subscription(Subscription&& other);
		STORMANCER_DLL_API Subscription& operator=(Subscription& other);
		STORMANCER_DLL_API virtual ~Subscription();

	public:
		STORMANCER_DLL_API bool subscribed() const;
		STORMANCER_DLL_API void unsubscribe();

	private:
		Action<> _unsubscribe;
		bool _subscribed = true;
	};

	using Subscription_ptr = std::shared_ptr<Subscription>;
};
