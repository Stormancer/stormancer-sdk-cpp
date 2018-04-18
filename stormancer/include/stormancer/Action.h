#pragma once

#include "stormancer/headers.h"

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename... TParams>
	class Action
	{
	public:

		using TFunction = std::function<void(TParams...)>;
		using TContainer = std::list<TFunction>;
		using TIterator = typename TContainer::iterator;
		using TAction = Action<TParams...>;

#pragma region public_methods

		Action()
		{
		}

		Action(TFunction f)
		{
			_functions.push_back(f);
		}

		virtual ~Action()
		{
		}

		Action(const TAction& other)
			: _functions(other._functions)
		{
		}

		Action(TAction&& other)
			: _functions(std::move(other._functions))
		{
		}

		TAction& operator=(const TAction& other)
		{
			_functions = other._functions;
			return *this;
		}

		TAction& operator=(TFunction f)
		{
			_functions.clear();
			_functions.push_back(f);
			return *this;
		}

		TAction& operator+=(TFunction f)
		{
			_functions.push_back(f);
			return *this;
		}

		const TAction& operator()(TParams... data) const
		{
			auto functionsCopy = _functions; // copy _functions because f can erase itself from the _functions
			for (auto f : functionsCopy)
			{
				f(data...);
			}
			return *this;
		}

		TIterator push_front(TFunction f)
		{
			return _functions.insert(_functions.begin(), f);
		}

		TIterator push_back(TFunction f)
		{
			return _functions.insert(_functions.end(), f);
		}

		void erase(TIterator it)
		{
			_functions.erase(it);
		}

		void clear()
		{
			_functions.clear();
		}

#pragma endregion

	private:

#pragma region private_members

		TContainer _functions;

#pragma endregion
	};

	/// Aggregates procedure pointers to be run simultaneously.
	template<>
	class Action<void>
	{
	public:

		using TFunction = std::function<void()>;
		using TContainer = std::list<TFunction>;
		using TIterator = TContainer::iterator;
		using TAction = Action<void>;

#pragma region public_methods

		Action()
		{
		}

		Action(TFunction f)
		{
			_functions.push_back(f);
		}

		virtual ~Action()
		{
		}

		Action(const TAction& other)
			: _functions(other._functions)
		{
		}

		Action(TAction&& other)
			: _functions(std::move(other._functions))
		{
		}

		TAction& operator=(const TAction& other)
		{
			_functions = other._functions;
			return *this;
		}

		TAction& operator=(TFunction f)
		{
			_functions.clear();
			_functions.push_back(f);
			return *this;
		}

		TAction& operator+=(TFunction f)
		{
			_functions.push_back(f);
			return *this;
		}

		const TAction& operator()() const
		{
			auto functionsCopy = _functions; // copy _functions because f can erase itself from the _functions
			for (auto f : functionsCopy)
			{
				f();
			}
			return *this;
		}

		TIterator push_front(TFunction f)
		{
			return _functions.insert(_functions.begin(), f);
		}

		TIterator push_back(TFunction f)
		{
			return _functions.insert(_functions.end(), f);
		}

		void erase(TIterator it)
		{
			_functions.erase(it);
		}

		void clear()
		{
			_functions.clear();
		}

#pragma endregion

	private:

#pragma region private_members

		TContainer _functions;

#pragma endregion
	};
};
