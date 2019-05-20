#include "stormancer/stdafx.h"
#include "stormancer/ClientFactory.h"
#include "stormancer/IClientFactory.h"
#include "stormancer/Client.h"

namespace Stormancer
{
	void IClientFactory::SetConfig(size_t id, std::function<std::shared_ptr<Configuration>()> configurator)
	{
		ClientFactory::SetConfig(id, configurator);
	}

	std::shared_ptr<IClient> IClientFactory::GetClient(size_t id)
	{
		return ClientFactory::GetClient(id);
	}

	void IClientFactory::ReleaseClient(size_t id)
	{
		ClientFactory::ReleaseClient(id);
	}

	void ClientFactory::SetConfig(size_t id, std::function<std::shared_ptr<Configuration>()> configurator)
	{
		_configurators[id] = configurator;
	}

	std::shared_ptr<IClient> ClientFactory::GetClient(size_t id)
	{
		std::lock_guard<std::mutex> lg(_mutex);

		auto it = _clients.find(id);
		if (it == _clients.end())
		{
			auto confIt = _configurators.find(id);
			if (confIt == _configurators.end())
			{
				throw std::runtime_error("missing configuration");
			}
			auto config = confIt->second();
			auto client = Client::create(config);
			_clients[id] =  client;
			return client;
		}
		else
		{
			return it->second;
		}
	}

	void ClientFactory::ReleaseClient(size_t id)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		_clients.erase(id);
	}

	std::mutex ClientFactory::_mutex;
	std::unordered_map<size_t, std::shared_ptr<IClient>> ClientFactory::_clients;
	std::unordered_map<size_t, std::function<std::shared_ptr<Configuration>()>> ClientFactory::_configurators;
}
