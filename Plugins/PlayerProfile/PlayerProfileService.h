#pragma once

#include "stormancer/headers.h"
#include "stormancer/IService.h"

namespace Stormancer
{
	template<class T>
	class PlayerProfileService : public IService
	{
	public:

#pragma region public_methods

		PlayerProfileService()
		{
		}

		void setScene(Scene* scene) override
		{
			_scene = scene;
		}

		pplx::task<T> get()
		{
			std::shared_ptr<RpcService> rpc = _scene->dependencyResolver()->template resolve<RpcService>();
			return rpc->rpc<T>("profiles.getsingle");
		}

		pplx::task<void> reset()
		{
			auto rpc = _scene->dependencyResolver()->template resolve<RpcService>();
			return rpc->rpc<void>("profiles.delete", 0);
		}

#pragma endregion

	private:

#pragma region private_members

		Scene* _scene = nullptr;

#pragma endregion
	};
}
