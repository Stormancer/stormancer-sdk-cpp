#pragma once
#include "headers.h"
#include "Scene.h"

namespace Stormancer
{
	class RpcService
	{
	public:
		RpcService(Scene& scene);
		virtual ~RpcService();

		// TODO

	private:
		const Scene& _scene;
	};
};
