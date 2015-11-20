#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename TParam = void>
	class Action
	{
		using TFunction = std::function<void(TParam)>;
		using TContainer = std::vector<TFunction>;
		using TAction = Action<TParam>;

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
		Action(const TAction& other)
			: _functions(other._functions)
		{
		}

		Action(TAction&& other)
		{
			_functions.reserve(other._functions.size());
			for (uint32 i = 0; i < other._functions.size(); i++)
			{
				_functions.push_back(other._functions[i]);
			}
		}

		TAction& operator=(const TAction& other)
		{
			_functions.reserve(other._functions.size());
			for (uint32 i = 0; i < other._functions.size(); i++)
			{
				_functions.push_back(other._functions[i]);
			}
			return *this;
		}

	public:
		TAction& operator=(TFunction function)
		{
			_functions.clear();
			_functions.push_back(function);
			return *this;
		}

		TAction& operator+=(TFunction function)
		{
			_functions.push_back(function);
			return *this;
		}

		const TAction& operator()(TParam data) const
		{
			size_t sz = _functions.size();
			for (uint32 i = 0; i < sz; i++)
			{
				auto f = _functions.at(i);
				f(data);
			}
			return *this;
		}

	private:
		TContainer _functions;
	};

	/// Aggregates procedure pointers to be run simultaneously.
	template<>
	class Action<void>
	{
		using TFunction = std::function<void(void)>;
		using TContainer = std::vector<TFunction>;
		using TAction = Action<>;

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
		Action(const TAction& other)
		{
			_functions.reserve(other._functions.size());
			for (uint32 i = 0; i < other._functions.size(); i++)
			{
				_functions.push_back(other._functions[i]);
			}
		}

		Action(TAction&& right)
			: _functions(std::move(right._functions))
		{
		}

		TAction& operator=(const TAction& other)
		{
			_functions.reserve(other._functions.size());
			for (uint32 i = 0; i < other._functions.size(); i++)
			{
				_functions.push_back(other._functions[i]);
			}
			return *this;
		}

	public:
		TAction& operator=(TFunction function)
		{
			_functions.clear();
			_functions.push_back(function);
			return *this;
		}

		TAction& operator+=(TFunction function)
		{
			_functions.push_back(function);
			return *this;
		}

		const TAction& operator()() const
		{
			size_t sz = _functions.size();
			for (uint32 i = 0; i < sz; i++)
			{
				auto f = _functions.at(i);
				f();
			}
			return *this;
		}

	private:
		TContainer _functions;
	};
};
