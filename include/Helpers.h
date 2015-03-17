#pragma once
#include "stdafx.h"

namespace Stormancer
{
	namespace Helpers
	{
		template<typename T>
		std::string mapKeys(std::map<std::string, T> map);

		std::string vectorJoin(std::vector<std::string> vector, std::string glue = "");
	};
};
