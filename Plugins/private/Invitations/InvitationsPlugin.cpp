#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Invitations/InvitationsPlugin.h"

#include "stormancer/Scene.h"
#include "Invitations/Invitations.h"
#include "stormancer/Logger/ILogger.h"

namespace Stormancer
{
	void InvitationsPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.invitations");
			if (name.length() > 0)
			{
				builder.registerDependency<InvitationsService, Scene, ILogger>().singleInstance();
			}
		}
	}
}
