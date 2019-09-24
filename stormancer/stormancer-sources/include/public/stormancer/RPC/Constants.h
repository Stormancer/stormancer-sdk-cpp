#pragma once

#include "stormancer/BuildConfig.h"
namespace Stormancer
{
	namespace rpc
	{
		constexpr auto pluginName = "stormancer.plugins.rpc";;
		constexpr auto serviceName = "rpcService";
		constexpr auto version = "1.1.0";
		constexpr auto nextRouteName = "stormancer.rpc.next";
		constexpr auto errorRouteName = "stormancer.rpc.error";
		constexpr auto completeRouteName = "stormancer.rpc.completed";
		constexpr auto cancellationRouteName = "stormancer.rpc.cancel";
	}
}