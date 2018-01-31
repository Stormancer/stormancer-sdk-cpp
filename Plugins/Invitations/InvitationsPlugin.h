#pragma once
#include <headers.h>
#include <IPlugin.h>

namespace Stormancer
{
	class InvitationsPlugin : public IPlugin
	{
	public:
		void sceneCreated(Scene* scene) override;
	};
}