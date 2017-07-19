#pragma once

#include "headers.h"

namespace Stormancer
{
	/// Represents an empty network message
	struct EmptyDto
	{
	public:
	
		/// Constructor.
		STORMANCER_DLL_API EmptyDto();
		
		/// Destructor.
		STORMANCER_DLL_API virtual ~EmptyDto();

		/// MessagePack deserialization.
		void msgpack_unpack(msgpack::object const& o);
	};
};
