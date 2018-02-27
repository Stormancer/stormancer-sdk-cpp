#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class FriendsPlugin: public IPlugin
	{
	public:
		void sceneCreated(Scene* scene) override;
	};
}
