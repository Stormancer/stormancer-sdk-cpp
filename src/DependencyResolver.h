#pragma once
#include "headers.h"

namespace Stormancer
{
	class DependencyResolver
	{
	public:
		DependencyResolver(DependencyResolver* parent = nullptr);
		virtual ~DependencyResolver();

	public:
		template<typename T>
		T* resolve()
		{
			auto& t = typeid(T);
			auto h = t.hash_code();
			if (mapContains(_factories, h))
			{
				return (T*)(_factories[h](this));
			}
			else if (_parent)
			{
				return _parent->resolve<T>();
			}
			else
			{
				throw std::runtime_error(std::string("The dependency resolver can't build the object (") + t.name() + ")");
			}
		}

		template<typename T>
		void registerDependency(std::function<T*(DependencyResolver* resolver)> factory)
		{
			auto& t = typeid(T);
			auto h = t.hash_code();
			_factories[h] = factory;
		}

		template<typename T>
		void registerDependency(T* instance)
		{
			auto& t = typeid(T);
			auto h = t.hash_code();
			_factories[h] = [instance](DependencyResolver* resolver) {
				return instance;
			};
		}

	private:
		std::map<std::size_t, std::function<void*(DependencyResolver* resolver)>> _factories;
		DependencyResolver* _parent;
	};
};
