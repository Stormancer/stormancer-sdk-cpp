#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/StormancerTypes.h"
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace Stormancer
{
	class KeyStore
	{
	public:
		std::unordered_map<std::string, std::vector<byte>> keys; // byte key[256 / 8];

		std::vector<byte>& getKey(std::string keyId)
		{
			auto it = keys.find(keyId);
			if (it == keys.end())
			{
				throw std::runtime_error("Key not found");
			}
			else
			{
				return it->second;
			}
		}

		void deleteKey(std::string keyId)
		{
			keys.erase(keyId);
		}
	};
}
