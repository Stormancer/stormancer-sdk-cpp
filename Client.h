#pragma once
#include <map>
#include <string>
#include "Configuration/ClientConfiguration.h"

namespace Stormancer
{
	class Client
	{
	private:
		std::map<std::string, ISerializer&> _serializers;
		std::map<std::string, std::string> _metadata;

	public:
		Client(ClientConfiguration config);
		~Client();
	};
};
