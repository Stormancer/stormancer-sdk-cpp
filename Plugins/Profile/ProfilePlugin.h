#pragma once
#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class ProfilePlugin : public IPlugin
	{
	public:
		ProfilePlugin();
		~ProfilePlugin();
		void sceneCreated(Scene* scene) override;
	};
};
