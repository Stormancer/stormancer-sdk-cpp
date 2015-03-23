#include "libs.h"
#include "Helpers.h"

namespace Stormancer
{
	namespace Helpers
	{
		bool ensureSuccessStatusCode(int statusCode)
		{
			return (statusCode >= 200 && statusCode < 300);
		}

		template<typename T>
		vector<string> mapKeys(map<string, T> map)
		{
			auto vec = vector<string>();
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				vec.push_back(it->first);
			}
			return vec;
		}

		template<typename T, typename U>
		bool mapContains(map<T, U> map, T& key)
		{
			return (map.find(key) != map.end) ? true : false;
		}

		string vectorJoin(vector<string> vector, string glue)
		{
			stringstream ss;
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

		StringFormat::operator const char*() const
		{
			return stream.str().c_str();
		}
	};
};
