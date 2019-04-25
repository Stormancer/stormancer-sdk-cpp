#pragma once

#include <string>
#include <vector>
#include "stormancer/msgpack_define.h"

namespace Stormancer
{
	struct ServerDescription
	{
		/// <summary>
		/// Server Id
		/// </summary>
		std::string Id;

		/// <summary>
		/// Server display name
		/// </summary>
		std::string Name;

		/// <summary>
		/// Is selected.
		/// </summary>
		bool Selected;
		
		MSGPACK_DEFINE(Id, Name, Selected)
	};
}
