#pragma once
#include <string>

namespace Stormancer
{
	class ISerializer
	{
	public:
		ISerializer();
		virtual ~ISerializer();

		virtual std::string serialize(std::string data) = 0;
		virtual std::string deserialize(std::string bytes) = 0;

	public:
		const std::string name;
	};
};
