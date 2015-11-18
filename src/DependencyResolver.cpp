#include "stormancer.h"

namespace Stormancer
{
	DependencyResolver::DependencyResolver(DependencyResolver* parent)
		: _parent(parent)
	{
	}

	DependencyResolver::~DependencyResolver()
	{
	}
};
