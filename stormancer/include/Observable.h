#pragma once
#include "headers.h"
#include "IObservable.h"
#include "Action.h"

namespace Stormancer
{
	template<typename T>
	class Observable : public IObservable<T>
	{
	public:
		Observable(rxcpp::observable<T> observable)
			: _observable(observable)
		{
		}

		~Observable()
		{
		}

		std::weak_ptr<ISubscription> subscribe(std::function<void(T)> onNext, std::function<void(const char* errorMessage)> onError, std::function<void()> onComplete)
		{
			auto it = _subscriptions.insert(_subscriptions.end(), std::shared_ptr<ISubscription>());

			auto& isubscription = *it;

			auto onErrorWrapper = [onError](std::exception_ptr eptr) {
				try
				{
					if (eptr)
					{
						std::rethrow_exception(eptr);
					}
					else
					{
						throw std::runtime_error("Unknown exception occured in Observable::subscribe(onNext, onError, onComplete)");
					}
				}
				catch (const std::exception& ex)
				{
					onError(ex.what());
				}
			};

			auto onCompleteWrapper = [this, onComplete, it]() {
				_subscriptions.erase(it);
				onComplete();
			};

			auto subscription = _observable.subscribe(onNext, onErrorWrapper, onCompleteWrapper);

			isubscription.reset(new Subscription([subscription]() { subscription.unsubscribe(); }));

			return isubscription;
		}

		std::weak_ptr<ISubscription> subscribe(std::function<void(T)> onNext, std::function<void(const char* errorMessage)> onError)
		{
			auto it = _subscriptions.insert(_subscriptions.end(), std::shared_ptr<ISubscription>());

			auto& isubscription = *it;

			auto onErrorWrapper = [onError](std::exception_ptr eptr) {
				try
				{
					if (eptr)
					{
						std::rethrow_exception(eptr);
					}
					else
					{
						throw std::runtime_error("Unknown exception occured in Observable::subscribe(onNext, onError)");
					}
				}
				catch (const std::exception& ex)
				{
					onError(ex.what());
				}
			};

			auto onCompleteWrapper = [this, it]() {
				_subscriptions.erase(it);
			};

			auto subscription = _observable.subscribe(onNext, onErrorWrapper, onCompleteWrapper);

			isubscription.reset(new Subscription([subscription]() { subscription.unsubscribe(); }));

			return isubscription;
		}

		std::weak_ptr<ISubscription> subscribe(std::function<void(T)> onNext)
		{
			auto it = _subscriptions.insert(_subscriptions.end(), std::shared_ptr<ISubscription>());

			auto& isubscription = *it;

			auto onErrorWrapper = [](std::exception_ptr eptr) {
				try
				{
					if (eptr)
					{
						std::rethrow_exception(eptr);
					}
					else
					{
						throw std::runtime_error("Unknown exception");
					}
				}
				catch (const std::exception& ex)
				{
					ILogger::instance()->log(LogLevel::Error, "Observable::subscribe(onNext)", "Observable error" + std::string(ex.what()));
				}
			};

			auto onCompleteWrapper = [this, it]() {
				_subscriptions.erase(it);
			};

			auto subscription = _observable.subscribe(onNext, onErrorWrapper, onCompleteWrapper);

			isubscription.reset(new Subscription([subscription]() { subscription.unsubscribe(); }));

			return isubscription;
		}

		std::weak_ptr<ISubscription> subscribe(std::function<void(T)> onNext, std::function<void()> onComplete)
		{
			auto it = _subscriptions.insert(_subscriptions.end(), std::shared_ptr<ISubscription>());

			auto& isubscription = *it;

			auto onErrorWrapper = [](std::exception_ptr eptr) {
				try
				{
					if (eptr)
					{
						std::rethrow_exception(eptr);
					}
					else
					{
						throw std::runtime_error("Unknown exception");
					}
				}
				catch (const std::exception& ex)
				{
					ILogger::instance()->log(LogLevel::Error, "Observable::subscribe(onNext, onComplete)", "Observable error :" + std::string(ex.what()));
				}
			};

			auto onCompleteWrapper = [this, onComplete, it]() {
				_subscriptions.erase(it);
				onComplete();
			};

			auto subscription = _observable.subscribe(onNext, onErrorWrapper, onCompleteWrapper);

			isubscription.reset(new Subscription([subscription]() { subscription.unsubscribe(); }));

			return isubscription;
		}

		void destroy()
		{
			Stormancer::destroy(this);
		}

	private:
		rxcpp::observable<T> _observable;
		std::list<std::shared_ptr<ISubscription>> _subscriptions;
	};
};
