#pragma once

#include "headers.h"
#include <exception>
#include <stdexcept>

namespace Stormancer
{
	class PointerDeletedException : public std::runtime_error
	{
	public:

		PointerDeletedException()
			: std::runtime_error("pointer_deleted")
		{
		}

		PointerDeletedException(const std::string& message)
			: std::runtime_error(message)
		{
		}
	};
}
