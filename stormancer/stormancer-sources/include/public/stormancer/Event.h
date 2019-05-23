#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Subscription.h"
#include "stormancer/Utilities/Macros.h"
#include <list>
#include <memory>
#include <functional>

namespace Stormancer
{
	/// Event on which we can subscribe
	template<typename... TParams>
	class Event
	{
	public:

#pragma region public_aliases

		using TEvent = Event<TParams...>;
		using TFunction = std::function<void(TParams...)>;
		using TContainer = std::list<std::shared_ptr<TFunction>>;
		using TIterator = typename TContainer::iterator;
		using Subscription = std::shared_ptr<Subscription_impl>;

#pragma endregion

#pragma region public_methods

		Event() = default;

		Event(const TEvent& other)
			: _functions(other._functions)
		{
		}

		Event(TEvent&& other)
			: _functions(std::move(other._functions))
		{
		}

		TEvent& operator=(const TEvent& other)
		{
			_functions = other._functions;
			return *this;
		}

		/// Trigger the Event and call the subscribed callbacks with the given parameters.
		/// \param data Parameters to give to the subscribed callbacks.
		/// \returns The Event.
		TEvent& operator()(TParams... data)
		{
			auto functionsCopy = *_functions;
			for (auto f : functionsCopy)
			{
				if (f)
				{
					(*f)(data...);
				}
			}
			return *this;
		}

		/// Calls the callback when the Event is triggered and returns a Subscription that will unsubscribe from the Event when it is deleted.
		/// \warning Don't forget to keep the Subscription returned for keeping registered to the Event.
		/// \param callback Callback that will be called when the Event is triggered.
		/// \returns A Subscription that will unsubscribe from the Event when it is deleted.
		STORM_NODISCARD
		Subscription subscribe(TFunction callback)
		{
			auto it = _functions->insert(_functions->end(), std::make_shared<TFunction>(callback));
			std::weak_ptr<TContainer> wFunctions = _functions;
			return std::make_shared<Subscription_impl>([wFunctions, it]()
			{
				if (auto functions = wFunctions.lock())
				{
					functions->erase(it);
				}
			});
		}

#pragma endregion

	private:

#pragma region private_members

		std::shared_ptr<TContainer> _functions = std::make_shared<TContainer>();

#pragma endregion
	};



	/// Event<void> specialization
	template<>
	class Event<void>
	{
	public:

#pragma region public_aliases

		using TEvent = Event<void>;
		using TFunction = std::function<void(void)>;
		using TContainer = std::list<std::shared_ptr<TFunction>>;
		using TIterator = typename TContainer::iterator;
		using Subscription = std::shared_ptr<Subscription_impl>;

#pragma endregion

#pragma region public_methods

		Event() = default;

		Event(const TEvent& other)
			: _functions(other._functions)
		{
		}

		Event(TEvent&& other)
			: _functions(std::move(other._functions))
		{
		}

		TEvent& operator=(const TEvent& other)
		{
			_functions = other._functions;
			return *this;
		}

		TEvent& operator()()
		{
			auto functionsCopy = *_functions;
			for (auto& f : functionsCopy)
			{
				if (f)
				{
					(*f)();
				}
			}
			return *this;
		}

		/// Calls the callback when the Event is triggered and returns a Subscription that will unsubscribe from the Event when it is deleted.
		/// \warning Don't forget to keep the Subscription returned for keeping registered to the Event.
		/// \param callback Callback that will be called when the Event is triggered.
		/// \returns A Subscription that will unsubscribe from the Event when it is deleted.
		STORM_NODISCARD
		Subscription subscribe(TFunction f)
		{
			auto it = _functions->insert(_functions->end(), std::make_shared<TFunction>(f));
			std::weak_ptr<TContainer> wFunctions = _functions;
			return std::make_shared<Subscription_impl>([wFunctions, it]()
			{
				if (auto functions = wFunctions.lock())
				{
					functions->erase(it);
				}
			});
		}

#pragma endregion

	private:

#pragma region private_members

		std::shared_ptr<TContainer> _functions = std::make_shared<TContainer>();
		
#pragma endregion
	};
}
