#include "stormancer/stdafx.h"
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

	void DependencyResolver::registerDependencyInternal(const std::type_info& t, Registration& registration, bool singleInstance)
	{
		auto h = t.hash_code();
		registration.singleInstance = singleInstance;
		_factories[h] = registration;
	}

	std::shared_ptr<void> DependencyResolver::resolveInternal(const std::type_info& t)
	{
		auto h = t.hash_code();
		if (mapContains(_factories, h))
		{
			auto& registration = _factories[h];
			if (registration.singleInstance)
			{
				if (registration.instance == nullptr)
				{
					if (registration.factory)
					{
						registration.instance = registration.factory(this);
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
					return registration.factory(this);
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
