#include "libs.h"
#include "Core/IConnection.h"

namespace Stormancer
{
	IConnection::IConnection()
	{
	}

	IConnection::~IConnection()
	{
	}

	template<typename T>
	void IConnection::registerComponent(T component)
	{
	}

	template<typename T>
	T IConnection::getComponent()
	{
		return T();
	}
};
