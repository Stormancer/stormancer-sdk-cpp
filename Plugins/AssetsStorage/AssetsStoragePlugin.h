#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class AssetsStoragePlugin : public IPlugin
	{
	public:
		void sceneCreated(Scene* scene) override;
	};
};
