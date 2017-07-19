#pragma once
#include <headers.h>
#include <IPlugin.h>

namespace Stormancer
{
	class GameSessionPlugin : public IPlugin
	{
	public:
		void sceneCreated(Scene* scene) override;
	};
};
