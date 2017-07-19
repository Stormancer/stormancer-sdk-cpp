// SmokeTest.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include <stormancer.h>
#include <io.h>
#include <exception>

int main()
{
	auto configuration = Stormancer::Configuration::create("http://104.199.17.79", "3dduo", "dev-server");
	auto client = Stormancer::Client::create(configuration);

	client->getPublicScene("monitoring").then([](Stormancer::Scene_ptr scene) {
		return scene->connect().then([scene]() {
			return scene->dependencyResolver()->resolve<Stormancer::RpcService>()->rpc<void, std::string>("monitoring.statistics", "A5906335F3E9A51E2DB8F4CC1CD140CA2DF798BDF0BF13650777081A68474997");
		});
	}).then([](pplx::task<void> t) {
		try
		{
			t.get();
			std::cout << "success" << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << "failure: " << ex.what() << std::endl;
		}
	});

	std::cin.ignore();
}

