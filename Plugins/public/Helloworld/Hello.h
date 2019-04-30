#pragma once
#include "stormancer/Tasks.h"
#include "Core/ClientAPI.h"

namespace Helloworld
{
	class Hello: public Stormancer::ClientAPI<Hello>
	{
	public:
		Hello(std::weak_ptr<Stormancer::AuthenticationService> auth);
		pplx::task<std::string> world(std::string name);
	};
}