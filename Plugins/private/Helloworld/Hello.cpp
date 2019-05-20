#include "Helloworld/Hello.h"
#include "Helloworld/HelloService.h"

namespace Helloworld
{

	Hello::Hello(std::weak_ptr<Stormancer::AuthenticationService> auth) : Stormancer::ClientAPI<Hello>(auth) {}

	pplx::task<std::string> Hello::world(std::string name)
	{
		return this->getService<HelloService>("helloworld")
			.then([name](std::shared_ptr<HelloService> hello) 
		{
			return hello->world(name); 
		});
	}
}
