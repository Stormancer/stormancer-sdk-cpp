#pragma once

#include "stormancer/Tasks.h"
#include "Members/OrganizationsTypes.h"
#include <vector>
#include <string>
#include <memory>

namespace Stormancer
{
	class OrganizationsContainer;

	class Organizations
	{
	public:

		virtual ~Organizations() {}

		virtual pplx::task<std::shared_ptr<OrganizationsContainer>> initialize() = 0;

		virtual pplx::task<Organization> getOrganization(const std::string& organizationId) = 0;
		virtual pplx::task<std::vector<Organization>> getOrganizations(const std::string& nameContains, int size = 10, int skip = 0) = 0;
		virtual pplx::task<std::vector<Organization>> getUserOrganizations(const std::string& userId, bool showApplyingMembers = false) = 0;
		virtual pplx::task<std::vector<Member>> getMembers(const std::string& organizationId, bool showApplyingMembers = false) = 0;
		virtual pplx::task<Organization> createOrganization(const std::string& organizationName, const std::string& customData = "{}") = 0;
		virtual pplx::task<void> updateOrganizationCustomData(const std::string& organizationId, const std::string& customData)= 0;
		virtual pplx::task<void> deleteOrganization(const std::string& organizationId)= 0;
		virtual pplx::task<void> changeOrganizationOwner(const std::string& newOwnerId, const std::string& organizationId)= 0;
		virtual pplx::task<void> inviteInOrganization(const std::string& userId, const std::string& organizationId)= 0;
		virtual pplx::task<void> manageOrganizationInvitation(bool accept, const std::string& userId, const std::string& organizationId)= 0;
		virtual pplx::task<void> removeMember(const std::string& memberId, const std::string& organizationId)= 0;
		virtual pplx::task<void> createOrganizationRole(const Role& role, const std::string& organizationId)= 0;
		virtual pplx::task<void> deleteOrganizationRole(const std::string& roleName, const std::string& organizationId)= 0;
		virtual pplx::task<void> grantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)= 0;
		virtual pplx::task<void> ungrantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)= 0;
	};
}
