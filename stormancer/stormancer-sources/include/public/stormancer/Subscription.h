#pragma once

#include <functional>
#include <memory>

namespace Stormancer
{
	/// Subscription (execute destroy when destructor is called)
	class Subscription_impl
	{
	public:

		/// The destroy callback will be called when the Subscription is deleted.
		Subscription_impl(std::function<void(void)> destroy);

		/// Call the destroy callback when deleted
		~Subscription_impl();

	private:

		std::function<void(void)> _destroy;
	};

	using Subscription = std::shared_ptr<Subscription_impl>;
}
