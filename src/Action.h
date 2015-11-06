#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename TParam = void>
	class Action
	{
		using TFunction = std::function<void(TParam)>;
		using TVector = std::vector<TFunction>;
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
			: _functions(std::move(other._functions))
		{
		}

		TAction& operator=(const TAction& other)
		{
			_functions = other._functions;
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
			for (auto f : _functions)
			{
				f(data);
			}
			return *this;
		}

	private:
		TVector _functions;
	};

	/// Aggregates procedure pointers to be run simultaneously.
	template<>
	class Action<void>
	{
		using TFunction = std::function<void(void)>;
		using TVector = std::vector<TFunction>;
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
			: _functions(other._functions)
		{
		}

		Action(TAction&& right)
			: _functions(std::move(right._functions))
		{
		}

		TAction& operator=(const TAction& other)
		{
			_functions = other._functions;
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
			for (auto f : _functions)
			{
				f();
			}
			return *this;
		}

	private:
		TVector _functions;
	};
};
