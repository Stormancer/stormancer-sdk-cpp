#pragma once
#include "rxcpp/rx.hpp"

namespace Stormancer
{
	class OrganizationsService;
	class Organizations_Impl;
	class Scene;

	class OrganizationsContainer
	{
		friend class Organizations_Impl;

	public:

		OrganizationsContainer() = default;

		std::shared_ptr<OrganizationsService> service();

	private:

		std::weak_ptr<Scene> _scene;
		rxcpp::composite_subscription _connectionStateChangedSubscription;
	};
}
