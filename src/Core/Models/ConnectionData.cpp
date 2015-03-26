#include "stormancer.h"

namespace Stormancer
{
	ConnectionData::ConnectionData()
	{
	}

	ConnectionData::~ConnectionData()
	{
		delete UserData;
	}
};
