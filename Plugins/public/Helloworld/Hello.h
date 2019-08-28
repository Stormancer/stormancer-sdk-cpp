#pragma once

#include "stormancer/Tasks.h"
#include "Users/ClientAPI.hpp"

namespace Helloworld
{
	class Hello: public Stormancer::ClientAPI<Hello>
	{
	public:
		Hello(std::weak_ptr<Stormancer::Users::UsersApi> users);
		pplx::task<std::string> world(std::string name);
	};
}