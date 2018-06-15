#include "stormancer/stdafx.h"
#include "stormancer/AES/IAES.h"

namespace Stormancer
{
	uint16 IAES::ivSize()
	{
		return 96 / 8;
	}
}