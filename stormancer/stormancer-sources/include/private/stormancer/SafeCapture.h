#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Exceptions.h"
#include <memory>

#if __cplusplus >= 201703L
#define STRM_WEAK_FROM_THIS() this->weak_from_this()
#else
#define STRM_WEAK_FROM_THIS() Stormancer::GetWeakFromThis(this)
#endif

namespace Stormancer
{
	template<typename T>
	std::weak_ptr<T> GetWeakFromThis(T* t)
	{
		if (!t)
		{
			throw std::runtime_error("Bad pointer");
		}

		try
		{
#if __cplusplus >= 201703L
			return t->weak_from_this();
#else
			return t->shared_from_this();
#endif
		}
		catch (const std::bad_weak_ptr&)
		{
			return std::weak_ptr<T>();
		}
		catch (...)
		{
			throw;
		}
	}

	template<typename T>
	std::shared_ptr<T> LockOrThrow(std::weak_ptr<T> wPtr)
	{
		auto sPtr = wPtr.lock();
		if (!sPtr)
		{
			throw PointerDeletedException();
		}
		return sPtr;
	}
}
