#pragma once
#include "headers.h"

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename TParam = void>
	class Action
	{
		using TFunction = std::function<void(TParam)>;
		using TContainer = DataStructures::List<TFunction>;
		using TAction = Action<TParam>;

	public:
		Action()
		{
		}

		Action(TFunction function)
		{
			_functions.Insert(function, __FILE__, __LINE__);
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
			_functions.Clear(false, __FILE__, __LINE__);
			_functions.Insert(function, __FILE__, __LINE__);
			return *this;
		}

		TAction& operator+=(TFunction function)
		{
			_functions.Insert(function, __FILE__, __LINE__);
			return *this;
		}

		const TAction& operator()(TParam data) const
		{
			uint32 sz = _functions.Size();
			for (uint32 i = 0; i < sz; i++)
			{
				auto f = _functions.Get(i);
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
		using TContainer = DataStructures::List<TFunction>;
		using TAction = Action<>;

	public:
		Action()
		{
		}

		Action(TFunction function)
		{
			_functions.Insert(function, __FILE__, __LINE__);
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
			_functions.Clear(false, __FILE__, __LINE__);
			_functions.Insert(function, __FILE__, __LINE__);
			return *this;
		}

		TAction& operator+=(TFunction function)
		{
			_functions.Insert(function, __FILE__, __LINE__);
			return *this;
		}

		const TAction& operator()() const
		{
			uint32 sz = _functions.Size();
			for (uint32 i = 0; i < sz; i++)
			{
				auto f = _functions.Get(i);
				f();
			}
			return *this;
		}

	private:
		TContainer _functions;
	};
};
