#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/RequestContext.h"

namespace Stormancer
{
	/// Allows modules to add handlers to system requests.
	/// Object passed to the IRequestModule::register method to allow modules to add handlers to system requests.
	class RequestModuleBuilder
	{
	public:
		RequestModuleBuilder(std::function<void(byte, std::function<pplx::task<void>(RequestContext*)>)> addHandler);
		virtual ~RequestModuleBuilder();

		void service(byte msgId, std::function<pplx::task<void>(RequestContext*)> handler);

	private:
		std::function<void(byte, std::function<pplx::task<void>(RequestContext*)>)> _addHandler;
	};
};
