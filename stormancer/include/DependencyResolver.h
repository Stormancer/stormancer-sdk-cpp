#pragma once
#include "headers.h"

namespace Stormancer
{
	class DependencyResolver
	{
	public:
		DependencyResolver(DependencyResolver* parent = nullptr);
		virtual ~DependencyResolver();

	private:
		struct Registration
		{
		public:
			std::function<std::shared_ptr<void>(DependencyResolver* resolver)> factory;
			bool singleInstance;
			std::shared_ptr<void> instance;
		};

	public:
		template<typename T>
		std::shared_ptr<T> resolve()
		{
			auto& t = typeid(T);
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
					return std::static_pointer_cast<T>(registration->instance);
				}
				else
				{
					return std::static_pointer_cast<T>(registration->factory(this));
				}
			}
			else if (_parent)
			{
				return _parent->resolve<T>();
			}
			else
			{
				return std::shared_ptr<T>();
			}
		}

		template<typename T>
		void registerDependency(std::function<std::shared_ptr<T>(DependencyResolver* resolver)> factory, bool singleInstance = false)
		{
			auto& t = typeid(T);
			auto h = t.hash_code();
			Registration registration;
			registration.factory = factory;
			registration.singleInstance = singleInstance;


			_factories[h] = registration;

		}

		template<typename T>
		void registerDependency(std::shared_ptr<T> instance)
		{
			registerDependency<T>([instance](DependencyResolver* resolver) {
				return instance;
			});
		}

	private:
		std::map<std::size_t, Registration> _factories;
		DependencyResolver* _parent = nullptr;


	};
};
