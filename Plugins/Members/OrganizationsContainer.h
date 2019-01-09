#pragma once
#include "stormancer/headers.h"
#include "rxcpp/rx.hpp"

namespace Stormancer
{
	class OrganizationsService;
	class Organizations;
	class Scene;

	class OrganizationsContainer
	{
		friend class Organizations;

	public:

		OrganizationsContainer() = default;

		std::shared_ptr<OrganizationsService> service();

	private:

		std::weak_ptr<Scene> _scene;
		rxcpp::composite_subscription _connectionStateChangedSubscription;
	};
}
