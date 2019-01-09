#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "InvitationsPlugin.h"
#include "Invitations.h"
#include "stormancer/Logger/ILogger.h"

void Stormancer::InvitationsPlugin::sceneCreated(std::shared_ptr<Scene> scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("stormancer.invitations");
		if (name.length() > 0)
		{

			scene->dependencyResolver()->registerDependency<InvitationsService>([scene](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<InvitationsService>(scene, dr.lock()->resolve<ILogger>());
			}, true);
		}
	}
}
