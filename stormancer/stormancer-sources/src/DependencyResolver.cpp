#include "stormancer/stdafx.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/DependencyResolver.h"

namespace Stormancer
{
	DependencyResolver::DependencyResolver(std::weak_ptr<DependencyResolver> parent)
		: _parent(parent)
	{
	}

	DependencyResolver::~DependencyResolver()
	{
	}

	void DependencyResolver::registerDependencyInternal(const ctti::type_id_t& t, Registration& registration, bool singleInstance)
	{
		auto h = static_cast<uint64>(t.hash());
		registration.singleInstance = singleInstance;
		_factories[h] = registration;
	}

	std::shared_ptr<void> DependencyResolver::resolveInternal(const ctti::type_id_t& t)
	{
		auto h = static_cast<uint64>(t.hash());
		auto it = _factories.find(h);
		if (it != _factories.end())
		{
			auto& registration = it->second;
			if (registration.singleInstance)
			{
				if (registration.instance == nullptr)
				{
					if (registration.factory)
					{
						registration.instance = registration.factory(STRM_WEAK_FROM_THIS());
					}
				}
				if (registration.instance)
				{
					return registration.instance;
				}
			}
			else
			{
				if (registration.factory)
				{
					return registration.factory(STRM_WEAK_FROM_THIS());
				}
			}
		}

		auto parent = _parent.lock();
		if (parent)
		{
			return parent->resolveInternal(t);
		}
		else
		{
			return nullptr;
		}
	}
};
