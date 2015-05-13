#pragma once
#include "headers.h"

namespace Stormancer
{
	template<typename TParam = void>
	class Action
	{
		using TFunction = function < void(TParam) > ;

	public:
		Action()
		{
		}

		Action(const Action& other)
			: _functions(other._functions)
		{
		}

		Action(Action&& right)
			: _functions(move(right._functions))
		{
		}

		virtual ~Action()
		{
			for (auto* f : _functions)
			{
				delete f;
			}
		}

	public:
		Action& operator=(TFunction* function)
		{
			_functions.clear();
			_functions.push_back(function);
			return *this;
		}

		Action& operator+=(TFunction* function)
		{
			_functions.push_back(function);
			return *this;
		}

		Action& operator-=(TFunction* function)
		{
			auto it = find(_functions.begin(), _functions.end(), function);
			if (it != _functions.end())
			{
				_functions.erase(it);
			}
		}

		Action& operator()(TParam data)
		{
			for (auto* f : _functions)
			{
				(*f)(data);
			}
			return *this;
		}

	private:
		vector<TFunction*> _functions;
	};

	template<>
	class Action < void >
	{
		using TFunction = function < void(void) > ;

	public:
		Action()
		{
		}

		Action(const Action& other)
			: _functions(other._functions)
		{
		}

		Action(Action&& right)
			: _functions(move(right._functions))
		{
		}

		virtual ~Action()
		{
			for (auto* f : _functions)
			{
				delete f;
			}
		}

	public:
		Action& operator=(TFunction* function)
		{
			_functions.clear();
			_functions.push_back(function);
			return *this;
		}

		Action& operator+=(TFunction* function)
		{
			_functions.push_back(function);
			return *this;
		}

		Action& operator-=(TFunction* function)
		{
			auto it = find(_functions.begin(), _functions.end(), function);
			if (it != _functions.end())
			{
				_functions.erase(it);
			}
		}

		Action& operator()()
		{
			for (auto* f : _functions)
			{
				(*f)();
			}
			return *this;
		}

	private:
		vector<TFunction*> _functions;
	};
};
