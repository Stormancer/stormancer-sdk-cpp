#include "stormancer/stdafx.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/Exceptions.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <algorithm>
#include <exception>
#include <mutex>

namespace Stormancer
{
	struct RegistrationInternal
	{
		RegistrationInternal(const RegistrationData& data)
			: lifetime(data.lifetime)
			, factory(data.factory)
			, tag(data.scopeTagToMatch)
			, id(data.id)
		{}

		DependencyLifetime lifetime;
		std::function<std::shared_ptr<void>(const DependencyScope&)> factory;
		std::string tag;
		RegistrationId id;
	};

	class DependencyScopeImpl
	{
	public:
		DependencyScopeImpl(const ContainerBuilder& builder, std::string tag, std::shared_ptr<DependencyScopeImpl> parent, DependencyScope* outer)
			: _registrations(buildRegistrations(builder))
			, _tag(tag)
			, _hasParent(parent)
			, _parent(parent)
			, _outer(outer)
			, _baseId(builder._registrationCounter)
		{}

		void resolveInternal(uint64 typeHash, const DependencyScope& requestingScope, std::vector<std::shared_ptr<void>>& instances, bool resolveSingle)
		{
			auto parent = tryLockParent();
			auto localRegsIt = _registrations.find(typeHash);
			if (localRegsIt != _registrations.end())
			{
				auto& localRegs = localRegsIt->second;
				if (resolveSingle)
				{
					auto instance = instantiateRegistration(localRegs.back(), requestingScope, true);
					instances.push_back(instance);
					return;
				}
				for (const auto& reg : localRegs)
				{
					auto instance = instantiateRegistration(reg, requestingScope, false);
					if (instance)
					{
						instances.push_back(instance);
					}
				}
			}
			if (parent && (!resolveSingle || localRegsIt == _registrations.end()))
			{
				parent->resolveInternal(typeHash, requestingScope, instances, resolveSingle);
			}
		}

		DependencyScope beginLifetimeScope(std::string tag, std::function<void(ContainerBuilder&)> builder) const
		{
			ContainerBuilder scopeBuilder(_baseId);
			if (builder)
			{
				builder(scopeBuilder);
			}
			return DependencyScope(scopeBuilder, tag, _outer->_impl);
		}

		// Used when moving DependencyScope
		void changeOuter(DependencyScope* newOuter)
		{
			_outer = newOuter;
		}

	private:
		std::shared_ptr<void> instantiateRegistration(const RegistrationInternal& registration, const DependencyScope& requestingScope, bool throwOnNotFound)
		{
			switch (registration.lifetime)
			{
			case DependencyLifetime::InstancePerRequest:
				return registration.factory(requestingScope);
			case DependencyLifetime::InstancePerScope:
				return getInstanceForScope(registration, requestingScope);
			case DependencyLifetime::InstancePerMatchingScope:
			{
				auto* scope = requestingScope._impl->findMatchingScope(registration.tag);
				if (scope)
				{
					return getInstanceForScope(registration, *scope);
				}
				if (throwOnNotFound)
				{
					throw DependencyResolutionException("No matching scope found for dependency registered as InstancePerMatchingScope with tag " + registration.tag);
				}
				return nullptr;
			}
			case DependencyLifetime::SingleInstance:
				return getInstanceForScope(registration, *_outer);
			default:
				// Unhandled DependencyLifetime value, please update this switch!
				std::terminate();
			}
		}

		DependencyScope* findMatchingScope(const std::string& tag)
		{
			if (tag == _tag)
			{
				return this->_outer;
			}
			if (auto parent = tryLockParent())
			{
				return parent->findMatchingScope(tag);
			}
			return nullptr;
		}

		static std::shared_ptr<void> getInstanceForScope(const RegistrationInternal& reg, const DependencyScope& scope)
		{
			std::lock_guard<std::recursive_mutex> lg(scope._impl->_instancesMutex);

			auto& instancesMap = scope._impl->_localInstances;
			auto existingInstanceIt = instancesMap.find(reg.id);
			if (existingInstanceIt != instancesMap.end())
			{
				return existingInstanceIt->second;
			}

			auto instance = reg.factory(scope);
			instancesMap[reg.id] = instance;
			return instance;
		}

		static std::unordered_map<uint64, std::vector<RegistrationInternal>> buildRegistrations(const ContainerBuilder& builder)
		{
			std::unordered_map<uint64, std::vector<RegistrationInternal>> registrations;
			for (const auto& registration : builder._registrations)
			{
				for (auto type : registration.typesRegisteredAs)
				{
					registrations[type].emplace_back(registration);
				}
				// If as() was not called, register the dependency as its original type
				if (registration.typesRegisteredAs.empty())
				{
					registrations[registration.actualType].emplace_back(registration);
				}
			}
			return registrations;
		}

		std::shared_ptr<DependencyScopeImpl> tryLockParent()
		{
			auto parent = _parent.lock();
			if (_hasParent && !parent)
			{
				throw DependencyResolutionException("Parent scope was deleted");
			}
			return parent;
		}

		const std::unordered_map<uint64, std::vector<RegistrationInternal>> _registrations;
		std::unordered_map<RegistrationId, std::shared_ptr<void>> _localInstances;

		std::string _tag;

		const bool _hasParent;
		std::weak_ptr<DependencyScopeImpl> _parent;
		// This pointer cannot be null, but it needs to be changed when std::moving _outer, which is why it is not a reference (and not const).
		DependencyScope* _outer;
		// When creating a child scope, this is the base Id for the child's registrations. This ensures uniqueness of registration Ids across a dependency tree branch.
		RegistrationId _baseId;

		std::recursive_mutex _instancesMutex;
	};

	DependencyScope::DependencyScope(const ContainerBuilder& builder, std::string tag, std::shared_ptr<DependencyScopeImpl> parent)
		: _impl(std::make_shared<DependencyScopeImpl>(builder, tag, parent, this))
	{}

	DependencyScope::DependencyScope(DependencyScope&& other)
		: _impl(std::move(other._impl))
	{
		_impl->changeOuter(this);
	}

	DependencyScope& DependencyScope::operator=(DependencyScope&& other)
	{
		_impl = std::move(other._impl);
		_impl->changeOuter(this);
		return *this;
	}


	DependencyScope DependencyScope::beginLifetimeScope(std::string tag, std::function<void(ContainerBuilder&)> builder) const
	{
		throwIfNotValid();

		return _impl->beginLifetimeScope(tag, builder);
	}

	std::shared_ptr<void> DependencyScope::resolveInternal(uint64 typeHash) const
	{
		throwIfNotValid();

		std::vector<std::shared_ptr<void>> instances;

		_impl->resolveInternal(typeHash, *this, instances, true);
		if (instances.empty())
		{
			throw DependencyResolutionException("Dependency not found");
		}
		return instances.front();
	}

	std::vector<std::shared_ptr<void>> DependencyScope::resolveAllInternal(uint64 typeHash) const
	{
		throwIfNotValid();

		std::vector<std::shared_ptr<void>> instances;

		_impl->resolveInternal(typeHash, *this, instances, false);
		return instances;
	}

	bool DependencyScope::isValid() const
	{
		if (_impl)
		{
			return true;
		}
		// Uninitialized (default-constructed)
		return false;
	}

	void DependencyScope::throwIfNotValid() const
	{
		if (!_impl)
		{
			throw DependencyResolutionException("This DependencyScope is invalid because it was default-constructed.");
		}
	}

	DependencyScope ContainerBuilder::build()
	{
		return DependencyScope(*this, "", nullptr);
	}

}