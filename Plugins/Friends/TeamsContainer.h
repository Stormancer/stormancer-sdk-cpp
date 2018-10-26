#pragma once
#include "stormancer/headers.h"

namespace Stormancer
{
	class TeamsService;
	class Teams;
	class Scene;

	class TeamsContainer
	{
		friend class Teams;

	public:

		TeamsContainer() = default;

		std::shared_ptr<TeamsService> service();

	private:

		std::weak_ptr<Scene> _scene;
		rxcpp::composite_subscription _connectionStateChangedSubscription;
	};
}
