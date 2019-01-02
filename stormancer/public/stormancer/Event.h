#pragma once

#include "stormancer/BuildConfig.h"

#include <mutex>
#include <list>
#include <memory>
#include <functional>
#include "stormancer/Tasks.h"

namespace Stormancer
{
	/// Aggregates procedures to be run simultaneously.
	template<typename TParam = void>
	class Event_impl
	{
	public:

		class Subscription_impl
		{
			using TFunction = std::function<void(TParam)>;
			using TContainer = std::list<TFunction>;
			using TIterator = typename TContainer::iterator;
			using TEvent = Event_impl<TParam>;

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

		using TFunction = std::function<void(TParam)>;
		using TContainer = std::list<std::shared_ptr<TFunction>>;
		using TIterator = typename TContainer::iterator;
		using TEvent = Event_impl<TParam>;

#pragma region public_methods

		Event_impl()
		{
		}

		Event_impl(TFunction f)
		{
			_functions.push_back(std::make_shared<TFunction>(f));
		}

		virtual ~Event_impl()
		{
		}

		Event_impl(const TEvent& other)
			: _functions(other._functions)
		{
		}

		Event_impl(TEvent&& other)
			: _functions(std::move(other._functions))
		{
		}

		TEvent& operator=(const TEvent& other)
		{
			_functions = other._functions;
			return *this;
		}

		TEvent& operator=(TFunction f)
		{
			_functions->clear();
			_functions->push_back(std::make_shared<TFunction>(f));
			return *this;
		}

		TEvent& operator+=(TFunction f)
		{
			_functions->push_back(std::make_shared<TFunction>(f));
			return *this;
		}

		const TEvent& operator()(TParam data, bool async = false) const
		{
			if (async)
			{
				pplx::task<void>([=]() {
					exec(data);
				});
			}
			else
			{
				exec(data);
			}

			return *this;
		}

		TIterator push_front(TFunction f)
		{
			return _functions->insert(_functions->begin(), std::make_shared<TFunction>(f));
		}

		TIterator push_back(TFunction f)
		{
			return _functions->insert(_functions->end(), std::make_shared<TFunction>(f));
		}

		void erase(TIterator it)
		{
			_functions->erase(it);
		}

		void clear()
		{
			_functions->clear();
		}

		Subscription subscribe(TFunction f)
		{
			auto ptr = std::make_shared<TFunction>(f);
			_functions->push_back(ptr);
			auto functions = _functions;
			return std::make_shared<Subscription_impl>([functions, ptr]() {

				functions->remove(ptr);
			});
		}

#pragma endregion

	private:

#pragma region private_methods

		inline void exec(const TParam& data) const
		{
			auto functionsCopy = *_functions; // copy _functions because f can erase itself from the _functions
			for (auto f : functionsCopy)
			{
				(*f)(data);
			}
		}

#pragma endregion

#pragma region private_members

		std::shared_ptr<TContainer> _functions = std::make_shared<TContainer>();

#pragma endregion
	};



	/// Aggregates procedure pointers to be run simultaneously.
	template<>
	class Event_impl<void> : public std::enable_shared_from_this<Event_impl<void>>
	{
	private:

		class Operation
		{
			Event_impl<void>* _action;

		public:
			Operation(Event_impl<void>* action)
			{
				_action = action;
				_action->_exec = true;
			}
			~Operation()
			{
				_action->_exec = false;
			}
		};

	public:

		class Subscription_impl
		{
			using TFunction = std::function<void(void)>;
			using TContainer = std::list<TFunction>;
			using TIterator = typename TContainer::iterator;
			using TEvent = Event_impl<void>;
		public:
			Subscription_impl(std::function<void(void)> f)
				: _f(f)

			{
			}
			~Subscription_impl()
			{
				_f();
			}

		private:
			std::function<void(void)> _f;
		};

		using Subscription = std::shared_ptr<Subscription_impl>;

		using TFunction = std::function<void(void)>;
		using TContainer = std::list<std::shared_ptr<TFunction>>;
		using TIterator = typename TContainer::iterator;
		using TEvent = Event_impl<void>;

#pragma region public_methods

		Event_impl()
		{
		}

		Event_impl(TFunction f)
		{
			_functions->push_back(std::make_shared<TFunction>(f));
		}

		virtual ~Event_impl()
		{
			this->_functions->clear();
			this->_operationQueue.clear();
		}

		Event_impl(const TEvent& other)
			: _functions(other._functions)
		{
		}

		Event_impl(TEvent&& other)
			: _functions(std::move(other._functions))
		{
		}

		TEvent& operator=(const TEvent& other)
		{
			_functions = other._functions;
			return *this;
		}

		TEvent& operator=(TFunction f)
		{
			_functions->clear();
			_functions->push_back(std::make_shared<TFunction>(f));
			return *this;
		}

		TEvent& operator+=(TFunction f)
		{
			_functions->push_back(std::make_shared<TFunction>(f));
			return *this;
		}

		const TEvent& operator()(bool async = false)
		{
			if (async)
			{
				pplx::task<void>([=]() {
					exec();
				});
			}
			else
			{
				exec();
			}

			return *this;
		}

		TIterator push_front(TFunction f)
		{
			return _functions->insert(_functions->begin(), std::make_shared<TFunction>(f));
		}

		TIterator push_back(TFunction f)
		{
			return _functions->insert(_functions->end(), std::make_shared<TFunction>(f));
		}

		void erase(TIterator it)
		{
			_functions->erase(it);
		}

		void clear()
		{
			_functions->clear();
		}

		Subscription subscribe(TFunction f)
		{
			auto ptr = std::make_shared<TFunction>(f);
			addPendingOperation([ptr](std::shared_ptr<Event_impl<void>> that) {
				that->_functions->push_back(ptr);
			});
			std::weak_ptr<Event_impl<void>> wThat = this->shared_from_this();

			return std::make_shared<Subscription_impl>([ptr, wThat]() {

				if (auto that = wThat.lock())
				{
					that->addPendingOperation([ptr](std::shared_ptr<Event_impl<void>> that) {
						that->_functions->remove(ptr);
					});
				}
			});

		}
#pragma endregion

	private:

#pragma region private_methods

		void addPendingOperation(std::function<void(std::shared_ptr<Event_impl<void>>)> op)
		{
			{
				std::lock_guard<std::mutex> lock(_opMutex);
				this->_operationQueue.push_back(op);
			}
			runPendingOperations();
		}

		bool runPendingOperations()
		{
			std::lock_guard<std::mutex> lock(_opMutex);
			if (!_exec)
			{
				for (auto& op : _operationQueue)
				{
					op(this->shared_from_this());
				}
				_operationQueue.clear();
				return true;
			}
			else
			{
				return false;
			}
		}

		inline void exec()
		{

			runPendingOperations();
			{
				std::lock_guard<std::mutex> lock(_execMutex);
				Operation op(this);

				// copy _functions because f can erase itself from the _functions
				for (auto& f : *_functions)
				{
					(*f)();
				}
			}
			runPendingOperations();
		}

#pragma endregion

#pragma region private_members

		std::shared_ptr<TContainer> _functions = std::make_shared<TContainer>();

		bool _exec = false;
		std::vector<std::function<void(std::shared_ptr<Event_impl<void>>)>> _operationQueue;
		std::mutex _opMutex;
		std::mutex _execMutex;

#pragma endregion
	};


	template<typename TParam = void>
	class Event
	{
	public:
		Event()
		{
			_impl = std::make_shared<Event_impl<TParam>>();
		}
	private:
		std::shared_ptr<Event_impl<TParam>> _impl;

	public:
		using Subscription = std::shared_ptr<typename Event_impl<TParam>::Subscription_impl>;
		using TFunction = std::function<void(TParam)>;
		using TEvent = Event<TParam>;

		Subscription subscribe(TFunction f)
		{
			return _impl->subscribe(f);
		}

		const TEvent& operator()(TParam data, bool async = false)
		{
			(*_impl)(data, async);

			return *this;
		}

		void clear()
		{
			_impl->clear();
		}
	};

	template<>
	class Event<void>
	{
	public:
		Event()
		{
			_impl = std::make_shared<Event_impl<void>>();
		}
	private:
		std::shared_ptr<Event_impl<void>> _impl;

	public:
		using Subscription = std::shared_ptr<typename Event_impl<void>::Subscription_impl>;
		using TFunction = std::function<void(void)>;
		using TEvent = Event<void>;

		Subscription subscribe(TFunction f)
		{
			return _impl->subscribe(f);
		}

		const TEvent& operator()(bool async = false)
		{
			(*_impl)(async);

			return *this;
		}

		void clear()
		{
			_impl->clear();
		}
	};

	template<typename TParam>
	using Action2 = Event<TParam>;
}
