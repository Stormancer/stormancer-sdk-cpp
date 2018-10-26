#pragma once
#include "stormancer/headers.h"
#include "TeamsTypes.h"

namespace Stormancer
{
	class TeamsContainer;
	class AuthenticationService;

	class Teams : public std::enable_shared_from_this<Teams>
	{
	public:

		Teams() = default;

		pplx::task<std::shared_ptr<TeamsContainer>> initialize(std::string teamSceneName);
		pplx::task<Team> getTeam(const std::string& teamId);
		pplx::task<Team> getTeamByName(const std::string& teamName);
		pplx::task<std::vector<TeamMember>> getMembers(const std::string& teamId, bool showApplyingMembers = false);
		pplx::task<Team> createTeam(const std::string& teamName, const std::string& customData = "{}");
		pplx::task<void> updateTeamCustomData(const std::string& teamId, const std::string& customData);
		pplx::task<void> deleteTeam(const std::string& teamId);
		pplx::task<void> changeTeamOwner(const std::string& newOwnerId, const std::string& teamId);
		pplx::task<void> inviteInTeam(const std::string& userId, const std::string& teamId);
		pplx::task<void> manageTeamInvitation(bool accept, const std::string& userId, const std::string& teamId);
		pplx::task<void> removeMember(const std::string& memberId, const std::string& teamId);
		pplx::task<void> createTeamRole(const Role& role, const std::string& teamId);
		pplx::task<void> deleteTeamRole(const std::string& roleName, const std::string& teamId);
		pplx::task<void> grantTeamRole(const std::string& roleName, const std::string& userId, const std::string& teamId);
		pplx::task<void> ungrantTeamRole(const std::string& roleName, const std::string& userId, const std::string& teamId);

	private:

		pplx::task<std::shared_ptr<TeamsContainer>> _teamsContainerTask;
		std::weak_ptr<AuthenticationService> _auth;
	};
}
