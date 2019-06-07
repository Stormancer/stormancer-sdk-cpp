#pragma once

#include <functional>
#include <memory>

namespace Stormancer
{
	/// Subscription (execute destroy when destructor is called)
	class Subscription_impl
	{
	public:

		/// The callback will be called when the Subscription is deleted.
		Subscription_impl(std::function<void(void)> callback);

		/// Call the callback when deleted
		~Subscription_impl();

		/// Unsubscribe and reset the callback
		void unsubscribe();

	private:

		// internal callback
		std::function<void(void)> _callback;
	};

	using Subscription = std::shared_ptr<Subscription_impl>;
}
