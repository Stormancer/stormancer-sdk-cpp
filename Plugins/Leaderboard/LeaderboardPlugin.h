#pragma once
#include <headers.h>
#include <cpprest\json.h>
#include <IPlugin.h>
#include <Scene.h>

namespace Stormancer
{
	class LeaderboardPlugin : public IPlugin
	{
	public:

		void sceneCreated(Scene* scene) override;
	};
}
