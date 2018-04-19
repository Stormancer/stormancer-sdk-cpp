#pragma once

#include "headers.h"
#include "stormancer/CustomExceptions.h"

#define STRM_SAFE_CAPTURE(...) Stormancer::createSafeCapture(this->shared_from_this(), __VA_ARGS__)

namespace Stormancer
{
	template<typename TClass, typename TCallable>
	class SafeCapture
	{
	private:

		std::weak_ptr<TClass> _weakPtr;
		TCallable _callable;

	public:

		SafeCapture(std::weak_ptr<TClass> weakPtr, TCallable callable)
			: _weakPtr(weakPtr)
			, _callable(callable)
		{
		}

		template<class... Args>
		auto operator()(Args&&...args) const -> decltype(_callable(std::forward<Args>(args)...))
		{
			auto sharedPtr = _weakPtr.lock();
			if (!sharedPtr)
			{
				throw PointerDeletedException();
			}
			return _callable(std::forward<Args>(args)...);
		}
	};

	template<typename TClass, typename TCallable>
	auto createSafeCapture(std::weak_ptr<TClass> weakPtr, TCallable callable) -> SafeCapture<TClass, TCallable>
	{
		return SafeCapture<TClass, TCallable>(weakPtr, callable);
	}

	template<typename TClass, typename TCallable>
	auto createSafeCapture(std::shared_ptr<TClass> sharedPtr, TCallable callable) -> SafeCapture<TClass, TCallable>
	{
		return SafeCapture<TClass, TCallable>(std::weak_ptr<TClass>(sharedPtr), callable);
	}
}
