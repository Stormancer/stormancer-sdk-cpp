#pragma once
#include "stormancer/headers.h"
#include "OrganizationsTypes.h"
#include "pplx/pplxtasks.h"

namespace Stormancer
{
	class Scene;

	class OrganizationsService
	{
	public:

#pragma region public_methods

		OrganizationsService(std::shared_ptr<Scene> scene);
		
		pplx::task<Organization> getOrganization(const std::string& organizationId);
		pplx::task<Organization> getOrganizationByName(const std::string& organizationName);
		pplx::task<std::vector<Organization>> getUserOrganizations(const std::string& userId, bool showApplyingMembers = false);
		pplx::task<std::vector<Member>> getMembers(const std::string& organizationId, bool showApplyingMembers = false);
		pplx::task<Organization> createOrganization(const std::string& organizationName, const std::string& customData = "{}");
		pplx::task<void> updateOrganizationCustomData(const std::string& organizationId, const std::string& customData);
		pplx::task<void> deleteOrganization(const std::string& organizationId);
		pplx::task<void> changeOrganizationOwner(const std::string& newOwnerId, const std::string& organizationId);
		pplx::task<void> inviteInOrganization(const std::string& userId, const std::string& organizationId);
		pplx::task<void> manageOrganizationInvitation(bool accept, const std::string& userId, const std::string& organizationId);
		pplx::task<void> removeMember(const std::string& memberId, const std::string& organizationId);
		pplx::task<void> createOrganizationRole(const Role& role, const std::string& organizationId);
		pplx::task<void> deleteOrganizationRole(const std::string& roleName, const std::string& organizationId);
		pplx::task<void> grantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId);
		pplx::task<void> ungrantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId);

#pragma endregion

	private:

#pragma region
	
		std::weak_ptr<Scene> _scene;

#pragma endregion
	};
}
