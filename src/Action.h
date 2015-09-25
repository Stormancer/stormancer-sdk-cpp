#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename TParam = void>
	class Action
	{
		using TFunction = std::function < void(TParam) >;

	public:
		Action()
		{
		}

		Action(TFunction function)
			: _functions{ function }
		{
		}

		virtual ~Action()
		{
		}

	public:
		Action(const Action<TParam>& other)
			: _functions(other._functions)
		{
		}

		Action(Action<TParam>&& other)
			: _functions(std::move(other._functions))
		{
		}

		Action<TParam>& operator=(const Action<TParam>& other)
		{
			_functions = other._functions;
			return *this;
		}

	public:
		Action<TParam>& operator=(TFunction function)
		{
			_functions.clear();
			_functions.push_back(function);
			return *this;
		}

		Action<TParam>& operator+=(TFunction function)
		{
			_functions.push_back(function);
			return *this;
		}

		/*
		Action<TParam>& operator-=(TFunction function)
		{
			auto it = find(_functions.begin(), _functions.end(), function);
			if (it != _functions.end())
			{
				_functions.erase(it);
			}
		}

		Action<TParam>& operator-=(TFunction& function)
		{
			auto it = find(_functions.begin(), _functions.end(), function);
			if (it != _functions.end())
			{
				_functions.erase(it);
			}
		}
		*/

		const Action<TParam>& operator()(TParam data) const
		{
			for (auto f : _functions)
			{
				f(data);
			}
			return *this;
		}

	private:
		std::vector<TFunction> _functions;
	};

	/// Aggregates procedure pointers to be run simultaneously.
	template<>
	class Action < void >
	{
		using TFunction = std::function < void(void) >;

	public:
		Action()
		{
		}

		Action(TFunction function)
		{
			_functions.push_back(function);
		}

		virtual ~Action()
		{
		}

	public:
		Action(const Action<>& other)
			: _functions(other._functions)
		{
		}

		Action(Action<>&& right)
			: _functions(std::move(right._functions))
		{
		}

		Action<>& operator=(const Action<>& other)
		{
			_functions = other._functions;
			return *this;
		}

	public:
		Action<>& operator=(TFunction function)
		{
			_functions.clear();
			_functions.push_back(function);
			return *this;
		}

		Action<>& operator+=(TFunction function)
		{
			_functions.push_back(function);
			return *this;
		}

		/*
		Action<>& operator-=(TFunction function)
		{
			auto it = find(_functions.begin(), _functions.end(), function);
			if (it != _functions.end())
			{
				_functions.erase(it);
			}
		}

		Action<>& operator-=(TFunction& function)
		{
			auto it = find(_functions.begin(), _functions.end(), function);
			if (it != _functions.end())
			{
				_functions.erase(it);
			}
		}
		*/

		const Action<>& operator()() const
		{
			for (auto f : _functions)
			{
				f();
			}
			return *this;
		}

	private:
		std::vector<TFunction> _functions;
	};
};
