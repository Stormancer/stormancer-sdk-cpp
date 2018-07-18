#pragma once
#include "pplx/pplxtasks.h"
namespace Stormancer
{
	class Shutdown
	{
	public:
		~Shutdown();

		///Returns a cancellation token that gets cancelled when the shutdown static variable is destroyed;
		pplx::cancellation_token getShutdownToken();

		inline
			static Shutdown& instance()
		{
			return _instance;
		}
	private:
		static Shutdown _instance;
		pplx::cancellation_token_source _cts;
	};
}