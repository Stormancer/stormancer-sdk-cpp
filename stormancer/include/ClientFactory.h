#pragma once

#include "Client.h"

namespace Stormancer
{
	class ClientFactory
	{
	public:

		static void SetConfig(size_t id, std::function<std::shared_ptr<Configuration>()> configurator);
		static std::shared_ptr<Client> GetClient(size_t id);

		static void ReleaseClient(size_t id);

	private:
		static std::mutex _mutex;
		static std::unordered_map<size_t, std::shared_ptr<Client>> _clients;
		static std::unordered_map<size_t, std::function<std::shared_ptr<Configuration>()>> _configurators;
	};
}