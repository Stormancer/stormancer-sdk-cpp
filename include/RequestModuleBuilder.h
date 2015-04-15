#pragma once
#include "headers.h"
#include "RequestContext.h"

namespace Stormancer
{
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
