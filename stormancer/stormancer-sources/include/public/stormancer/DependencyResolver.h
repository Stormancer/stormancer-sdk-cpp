#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/external/ctti/type_id.hpp"
#include <memory>
#include <functional>
#include <map>

namespace Stormancer
{
	class DependencyResolver : public std::enable_shared_from_this<DependencyResolver>
	{
	private:

#pragma region private_classes

		struct Registration
		{
		public:
			std::function<std::shared_ptr<void>(std::weak_ptr<DependencyResolver> resolver)> factory;
			bool singleInstance;
			std::shared_ptr<void> instance;
		};

#pragma endregion

	public:

#pragma region public_methods

		DependencyResolver(std::weak_ptr<DependencyResolver> parent = std::weak_ptr<DependencyResolver>());

		virtual ~DependencyResolver();

		template<typename T>
		std::shared_ptr<T> resolve()
		{
			auto t = ctti::type_id<T>();
			auto ptr = resolveInternal(t);

			if (ptr)
			{
				return std::static_pointer_cast<T>(ptr);
			}
			else
			{
				return nullptr;
			}
		}
		
		std::shared_ptr<void> resolveInternal(const ctti::type_id_t& t);

		template<typename T>
		void registerDependency(std::function<std::shared_ptr<T>(std::weak_ptr<DependencyResolver> resolver)> factory, bool singleInstance = false)
		{
			auto t = ctti::type_id<T>();
			Registration registration;
			registration.factory = factory;
			registerDependencyInternal(t, registration, singleInstance);
		}

		template<typename T>
		void registerDependency(std::shared_ptr<T> instance)
		{
			registerDependency<T>([=](std::weak_ptr<DependencyResolver>) {
				return instance;
			});
		}

#pragma endregion

	private:

#pragma region private_methods

		void registerDependencyInternal(const ctti::type_id_t& t, Registration& registration, bool singleInstance = false);

#pragma endregion

#pragma region private_members

		std::map<uint64, Registration> _factories;
		std::weak_ptr<DependencyResolver> _parent;

#pragma endregion
	};
};
