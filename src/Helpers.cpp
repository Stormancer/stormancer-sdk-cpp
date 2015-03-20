#include "Helpers.h"

namespace Stormancer
{
	namespace Helpers
	{
		template<typename T>
		std::vector<std::string> mapKeys(std::map<std::string, T> map)
		{
			auto vec = std::vector<std::string>();
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->first);
			}
			return vec;
		}

		std::string vectorJoin(std::vector<std::string> vector, std::string glue)
		{
			std::stringstream ss;
			for (size_t i = 0; i < vector.size(); ++i)
			{
				if (i != 0)
				{
					ss << glue;
				}
				ss << vector[i];
			}
			return ss.str();
		}

		template<typename T>
		StringFormat& StringFormat::operator << (T data)
		{
			stream << data;
			return *this;
		}

		template<typename T>
		void StringFormat::operator >> (T& data)
		{
			stream >> data;
		}

		string StringFormat::str() const
		{
			return stream.str();
		}

		StringFormat::operator string() const
		{
			return stream.str();
		}
	};
};
