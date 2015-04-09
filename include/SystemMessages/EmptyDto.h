#pragma once
#include "headers.h"

namespace Stormancer
{
	class EmptyDto
	{
	public:
		static EmptyDto& instance()
		{
			static EmptyDto sInstance;
			return sInstance;
		}

		EmptyDto()
		{
		}

		~EmptyDto()
		{
		}

		EmptyDto& operator=(const EmptyDto& other)
		{
			return *this;
		}

	public:
		const bool value = true;
	};
};
