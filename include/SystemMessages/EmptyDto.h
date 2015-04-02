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

	private:
		EmptyDto()
		{
		}

		~EmptyDto()
		{
		}

	public:
		const bool value = true;
	};
};
