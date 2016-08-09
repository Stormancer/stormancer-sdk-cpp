#pragma once

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename TParam = void>
	class Action
	{
	public:
		using TFunction = std::function<void(TParam)>;
		using TContainer = std::list<TFunction>;
		using TIterator = typename TContainer::iterator;
		using TAction = Action<TParam>;

	public:
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

	public:
		Action(const TAction& other)
			: _functions(other._functions)
		{
		}

		Action(TAction&& other)
			: _functions(std::move(other._functions))
		{
		}

	public:
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

		const TAction& operator()(TParam data, bool async = false) const
		{
			if (async)
			{
				pplx::task<void>([this, data]() {
					exec(data);
				});
			}
			else
			{
				exec(data);
			}

			return *this;
		}

	public:
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

	private:

		inline void exec(const TParam& data) const
		{
			auto functionsCopy = _functions; // copy _functions because f can erase itself from the _functions
			for (auto f : functionsCopy)
			{
				f(data);
			}
		}

	private:
		TContainer _functions;
	};

	/// Aggregates procedure pointers to be run simultaneously.
	template<>
	class Action<void>
	{
	public:
		using TFunction = std::function<void(void)>;
		using TContainer = std::list<TFunction>;
		using TIterator = TContainer::iterator;
		using TAction = Action<>;

	public:
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

	public:
		Action(const TAction& other)
			: _functions(other._functions)
		{
		}

		Action(TAction&& other)
			: _functions(std::move(other._functions))
		{
		}

	public:
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

		const TAction& operator()(bool async = false) const
		{
			if (async)
			{
				pplx::task<void>([this]() {
					exec();
				});
			}
			else
			{
				exec();
			}

			return *this;
		}

	public:
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

	private:
		inline void exec() const
		{
			auto functionsCopy = _functions; // copy _functions because f can erase itself from the _functions
			for (auto f : functionsCopy)
			{
				f();
			}
		}

	private:
		TContainer _functions;
	};
};
