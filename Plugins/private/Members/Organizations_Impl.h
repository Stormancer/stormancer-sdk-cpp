#pragma once
#include "Members/Organizations.h"
#include "Core/ClientAPI.h"
#include "OrganizationsTypes.h"

namespace Stormancer
{
	class OrganizationsContainer;
	class AuthenticationService;

	class Organizations_Impl : public ClientAPI<Organizations_Impl>, public Organizations
	{
	public:

		Organizations_Impl(std::shared_ptr<AuthenticationService> auth);

		pplx::task<std::shared_ptr<OrganizationsContainer>> initialize() override;

		pplx::task<Organization> getOrganization(const std::string& organizationId) override;
		pplx::task<std::vector<Organization>> getOrganizations(const std::string& nameContains, int size = 10, int skip = 0) override;
		pplx::task<std::vector<Organization>> getUserOrganizations(const std::string& userId, bool showApplyingMembers = false) override;
		pplx::task<std::vector<Member>> getMembers(const std::string& organizationId, bool showApplyingMembers = false) override;
		pplx::task<Organization> createOrganization(const std::string& organizationName, const std::string& customData = "{}") override;
		pplx::task<void> updateOrganizationCustomData(const std::string& organizationId, const std::string& customData) override;
		pplx::task<void> deleteOrganization(const std::string& organizationId) override;
		pplx::task<void> changeOrganizationOwner(const std::string& newOwnerId, const std::string& organizationId) override;
		pplx::task<void> inviteInOrganization(const std::string& userId, const std::string& organizationId) override;
		pplx::task<void> manageOrganizationInvitation(bool accept, const std::string& userId, const std::string& organizationId) override;
		pplx::task<void> removeMember(const std::string& memberId, const std::string& organizationId) override;
		pplx::task<void> createOrganizationRole(const Role& role, const std::string& organizationId) override;
		pplx::task<void> deleteOrganizationRole(const std::string& roleName, const std::string& organizationId) override;
		pplx::task<void> grantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId) override;
		pplx::task<void> ungrantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId) override;

	private:

		pplx::task<std::shared_ptr<OrganizationsContainer>> _organizationsContainerTask;
		std::weak_ptr<AuthenticationService> _auth;
	};
}
