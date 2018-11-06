#include "stormancer/headers.h"
#include "Organizations.h"
#include "OrganizationsContainer.h"
#include "OrganizationsService.h"
#include "stormancer/Scene.h"
#include "stormancer/Exceptions.h"
#include "../Authentication/AuthenticationService.h"

namespace Stormancer
{
	pplx::task<std::shared_ptr<OrganizationsContainer>> Organizations::initialize(std::string organizationSceneName)
	{
		auto auth = _auth.lock();

		if (!auth)
		{
			return pplx::task_from_exception<std::shared_ptr<OrganizationsContainer>>(std::runtime_error("Authentication service destroyed."));
		}

		std::weak_ptr<Organizations> wThat = this->shared_from_this();
		_organizationsContainerTask = auth->getSceneForService("stormancer.organizations", organizationSceneName)
			.then([wThat](std::shared_ptr<Scene> scene)
		{
			auto that = wThat.lock();
			if (!that)
			{
				throw PointerDeletedException("Authentication service destroyed.");
			}

			auto container = std::make_shared<OrganizationsContainer>();
			container->_scene = scene;
			container->_connectionStateChangedSubscription = scene->getConnectionStateChangedObservable()
				.subscribe([wThat](ConnectionState s)
			{
				if (s == ConnectionState::Disconnected)
				{
					if (auto that = wThat.lock())
					{
						that->_organizationsContainerTask = pplx::task_from_result<std::shared_ptr<OrganizationsContainer>>(nullptr);
					}
				}
			});
			return container;
		});
		return _organizationsContainerTask;
	}

	pplx::task<Organization> Organizations::getOrganization(const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getOrganization(organizationId);
		});
	}

	pplx::task<Organization> Organizations::getOrganizationByName(const std::string& organizationName)
	{
		return _organizationsContainerTask
			.then([organizationName](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getOrganizationByName(organizationName);
		});
	}

	pplx::task<std::vector<Organization>> Organizations::getUserOrganizations(const std::string& userId, bool showApplyingMembers)
	{
		return _organizationsContainerTask
			.then([userId, showApplyingMembers](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getUserOrganizations(userId, showApplyingMembers);
		});
	}

	pplx::task<std::vector<Member>> Organizations::getMembers(const std::string& organizationId, bool showApplyingMembers)
	{
		return _organizationsContainerTask
			.then([organizationId, showApplyingMembers](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getMembers(organizationId, showApplyingMembers);
		});
	}

	pplx::task<Organization> Organizations::createOrganization(const std::string& organizationName, const std::string& customData)
	{
		return _organizationsContainerTask
			.then([organizationName, customData](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->createOrganization(organizationName, customData);
		});
	}

	pplx::task<void> Organizations::updateOrganizationCustomData(const std::string& organizationId, const std::string& customData)
	{
		return _organizationsContainerTask
			.then([organizationId, customData](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->updateOrganizationCustomData(organizationId, customData);
		});
	}

	pplx::task<void> Organizations::deleteOrganization(const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->deleteOrganization(organizationId);
		});
	}

	pplx::task<void> Organizations::changeOrganizationOwner(const std::string& newOwnerId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([newOwnerId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->changeOrganizationOwner(newOwnerId, organizationId);
		});
	}

	pplx::task<void> Organizations::inviteInOrganization(const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->inviteInOrganization(userId, organizationId);
		});
	}

	pplx::task<void> Organizations::removeMember(const std::string& memberId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([memberId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->removeMember(memberId, organizationId);
		});
	}

	pplx::task<void> Organizations::manageOrganizationInvitation(bool accept, const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([accept, userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->manageOrganizationInvitation(accept, userId, organizationId);
		});
	}

	pplx::task<void> Organizations::createOrganizationRole(const Role& role, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([role, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->createOrganizationRole(role, organizationId);
		});
	}

	pplx::task<void> Organizations::deleteOrganizationRole(const std::string& roleName, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([roleName, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->deleteOrganizationRole(roleName, organizationId);
		});
	}

	pplx::task<void> Organizations::grantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([roleName, userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->grantOrganizationRole(roleName, userId, organizationId);
		});
	}

	pplx::task<void> Organizations::ungrantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([roleName, userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->grantOrganizationRole(roleName, userId, organizationId);
		});
	}
}
