#include "stormancer.h"

namespace Stormancer
{
	IConnection::IConnection()
	{
	}

	IConnection::~IConnection()
	{
	}

	template<typename T>
	void IConnection::registerComponent(T* component)
	{
		_components[typeid(T)] = dynamic_cast<void*>(component);
	}

	template<typename T>
	bool IConnection::getComponent(T* component)
	{
		type_info tid = typeid(T);
		if (Helpers::mapContains(_components, tid))
		{
			if (component != nullptr)
			{
				component = dynamic_cast<T*>(_components[tid]);
			}
			return true;
		}

		component = nullptr;
		return false;
	}
};
