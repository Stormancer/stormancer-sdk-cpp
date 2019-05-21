#pragma once

#include "stormancer/IPlugin.h"
#include "stormancer/IClient.h"
#include <memory>

namespace Stormancer
{
	class AuthenticationPlugin : public IPlugin
	{
	public:
		void registerClientDependencies(ContainerBuilder& builder) override;
		void clientDisconnecting(std::shared_ptr<IClient> client) override;
	};
};
