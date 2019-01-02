#pragma once

#include "stormancer/BuildConfig.h"
#include <memory>
#include <functional>
#include <map>
#include <typeinfo>

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
			auto& t = typeid(T);
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
		
		std::shared_ptr<void> resolveInternal(const std::type_info& t);

		template<typename T>
		void registerDependency(std::function<std::shared_ptr<T>(std::weak_ptr<DependencyResolver> resolver)> factory, bool singleInstance = false)
		{
			auto& t = typeid(T);
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

		void registerDependencyInternal(const std::type_info& t, Registration& registration, bool singleInstance = false);

#pragma endregion

#pragma region private_members

		std::map<std::size_t, Registration> _factories;
		std::weak_ptr<DependencyResolver> _parent;

#pragma endregion
	};
};
