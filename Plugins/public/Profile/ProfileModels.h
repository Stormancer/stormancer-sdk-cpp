#pragma once
#include <string>
#include <unordered_map>

namespace Stormancer
{
	struct Profile
	{
		std::unordered_map<std::string, std::string> data;
	};
}
