#pragma once
#include "headers.h"
#include "RequestContext.h"

namespace Stormancer
{
	/// Allows modules to add handlers to system requests.
	/// Object passed to the IRequestModule::register method to allow modules to add handlers to system requests.
	class RequestModuleBuilder
	{
	public:
		RequestModuleBuilder(function<void(byte, function<pplx::task<void>(RequestContext*)>)> addHandler);
		virtual ~RequestModuleBuilder();

		void service(byte msgId, function<pplx::task<void>(RequestContext*)> handler);

	private:
		function<void(byte, function<pplx::task<void>(RequestContext*)>)> _addHandler;
	};
};
