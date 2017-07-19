#include "stdafx.h"
#include "DependencyResolver.h"

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
			auto registration = &_factories[h];
			if (registration->singleInstance)
			{
				if (registration->instance == nullptr)
				{
					registration->instance = registration->factory(this);
				}
				return registration->instance;
			}
			else
			{
				return registration->factory(this);
			}
		}
		else
		{
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
	}
};
