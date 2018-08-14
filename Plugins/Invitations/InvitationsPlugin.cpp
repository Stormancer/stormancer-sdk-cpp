#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "InvitationsPlugin.h"
#include "Invitations.h"

void Stormancer::InvitationsPlugin::sceneCreated(Scene * scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("stormancer.invitations");
		if (name.length() > 0)
		{

			scene->dependencyResolver().lock()->registerDependency<InvitationsService>([scene](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<InvitationsService>(scene->shared_from_this(), dr.lock()->resolve<ILogger>());
			}, true);
		}
	}
}
