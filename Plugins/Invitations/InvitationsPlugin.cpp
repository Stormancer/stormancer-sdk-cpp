#include "stdafx.h"
#include "stormancer.h"
#include "InvitationsPlugin.h"
#include "Invitations.h"

void Stormancer::InvitationsPlugin::sceneCreated(Scene * scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("stormancer.invitations");
		if (name.length() > 0)
		{

			scene->dependencyResolver()->registerDependency<InvitationsService>([scene](DependencyResolver* dr) {
				return std::make_shared<InvitationsService>(scene->shared_from_this(), dr->resolve<ILogger>());
			}, true);
		}
	}
}
