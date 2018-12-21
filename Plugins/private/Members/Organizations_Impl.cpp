#include "Organizations_impl.h"
#include "OrganizationsContainer.h"
#include "OrganizationsService.h"
#include "stormancer/Scene.h"
#include "stormancer/Exceptions.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	Organizations_Impl::Organizations_Impl(std::shared_ptr<AuthenticationService> auth)
		: ClientAPI(auth)
		, _auth(auth)
	{
	}

	pplx::task<std::shared_ptr<OrganizationsContainer>> Organizations_Impl::initialize()
	{
		auto auth = _auth.lock();

		if (!auth)
		{
			return pplx::task_from_exception<std::shared_ptr<OrganizationsContainer>>(std::runtime_error("Authentication service destroyed."));
		}

		std::weak_ptr<Organizations_Impl> wOrganizations = this->shared_from_this();
		_organizationsContainerTask = auth->getSceneForService("stormancer.plugins.organizations")
			.then([wOrganizations](std::shared_ptr<Scene> scene)
		{
			auto organizations = wOrganizations.lock();
			if (!organizations)
			{
				throw PointerDeletedException("Authentication service destroyed.");
			}

			auto container = std::make_shared<OrganizationsContainer>();
			container->_scene = scene;
			container->_connectionStateChangedSubscription = scene->getConnectionStateChangedObservable()
				.subscribe([wOrganizations](ConnectionState s)
			{
				if (s == ConnectionState::Disconnected)
				{
					if (auto organizations = wOrganizations.lock())
					{
						organizations->_organizationsContainerTask = pplx::task_from_result<std::shared_ptr<OrganizationsContainer>>(nullptr);
					}
				}
			});
			return container;
		});
		return _organizationsContainerTask;
	}

	pplx::task<Organization> Organizations_Impl::getOrganization(const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getOrganization(organizationId);
		});
	}

	pplx::task<std::vector<Organization>> Organizations_Impl::getOrganizations(const std::string& nameContains, int size, int skip)
	{
		return _organizationsContainerTask
			.then([nameContains, size, skip](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getOrganizations(nameContains, size, skip);
		});
	}

	pplx::task<std::vector<Organization>> Organizations_Impl::getUserOrganizations(const std::string& userId, bool showApplyingMembers)
	{
		return _organizationsContainerTask
			.then([userId, showApplyingMembers](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getUserOrganizations(userId, showApplyingMembers);
		});
	}

	pplx::task<std::vector<Member>> Organizations_Impl::getMembers(const std::string& organizationId, bool showApplyingMembers)
	{
		return _organizationsContainerTask
			.then([organizationId, showApplyingMembers](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->getMembers(organizationId, showApplyingMembers);
		});
	}

	pplx::task<Organization> Organizations_Impl::createOrganization(const std::string& organizationName, const std::string& customData)
	{
		return _organizationsContainerTask
			.then([organizationName, customData](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->createOrganization(organizationName, customData);
		});
	}

	pplx::task<void> Organizations_Impl::updateOrganizationCustomData(const std::string& organizationId, const std::string& customData)
	{
		return _organizationsContainerTask
			.then([organizationId, customData](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->updateOrganizationCustomData(organizationId, customData);
		});
	}

	pplx::task<void> Organizations_Impl::deleteOrganization(const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->deleteOrganization(organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::changeOrganizationOwner(const std::string& newOwnerId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([newOwnerId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->changeOrganizationOwner(newOwnerId, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::inviteInOrganization(const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->inviteInOrganization(userId, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::removeMember(const std::string& memberId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([memberId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->removeMember(memberId, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::manageOrganizationInvitation(bool accept, const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([accept, userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->manageOrganizationInvitation(accept, userId, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::createOrganizationRole(const Role& role, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([role, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->createOrganizationRole(role, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::deleteOrganizationRole(const std::string& roleName, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([roleName, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->deleteOrganizationRole(roleName, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::grantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([roleName, userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->grantOrganizationRole(roleName, userId, organizationId);
		});
	}

	pplx::task<void> Organizations_Impl::ungrantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)
	{
		return _organizationsContainerTask
			.then([roleName, userId, organizationId](std::shared_ptr<OrganizationsContainer> organizationsContainer)
		{
			return organizationsContainer->service()->ungrantOrganizationRole(roleName, userId, organizationId);
		});
	}
}
