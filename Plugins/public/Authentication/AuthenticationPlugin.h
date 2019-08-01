#pragma once

#include "stormancer/IPlugin.h"
#include "stormancer/IClient.h"
#include <memory>

namespace Stormancer
{
	/// \deprecated Please use <c>Stormancer::Users::UsersPlugin</c> instead.
	class AuthenticationPlugin : public IPlugin
	{
	public:
		void registerClientDependencies(ContainerBuilder& builder) override;
		void clientDisconnecting(std::shared_ptr<IClient> client) override;
	};
}
