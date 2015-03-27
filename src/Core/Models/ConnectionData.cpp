#include "stormancer.h"

namespace Stormancer
{
	ConnectionData::ConnectionData()
		: UserData(nullptr)
	{
	}

	ConnectionData::~ConnectionData()
	{
		if (UserData != nullptr)
		{
			delete UserData;
		}
	}
};
