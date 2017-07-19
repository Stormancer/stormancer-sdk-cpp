#pragma once
#include "Client.h"

namespace Stormancer
{
	class IClientFactory
	{
	public:

		static void SetConfig(size_t id, std::function<std::shared_ptr<Configuration>()> configurator);
		static std::shared_ptr<Client> GetClient(size_t id);

		static void ReleaseClient(size_t id);
	};
}