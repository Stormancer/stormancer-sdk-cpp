#pragma once

#include "stormancer/BuildConfig.h"

#include <list>
#include <memory>
#include <functional>

namespace Stormancer
{
	// Subscription (execute destroy when destructor is called)
	class Subscription_impl
	{
	public:

		Subscription_impl(std::function<void(void)> destroy)
			: _destroy(destroy)
		{
		}

		~Subscription_impl()
		{
			_destroy();
		}

	private:

		std::function<void(void)> _destroy;
	};

	using Subscription = std::shared_ptr<Subscription_impl>;

	/// Aggregates procedures to be run simultaneously.
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



	/// Aggregates procedure pointers to be run simultaneously.
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
