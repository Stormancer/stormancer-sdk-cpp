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
		std::unordered_map<uint64, std::vector<byte>> keys; // byte key[256 / 8];

		std::vector<byte>& getKey(uint64 keyId)
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

		void deleteKey(uint64 keyId)
		{
			keys.erase(keyId);
		}
	};
}
