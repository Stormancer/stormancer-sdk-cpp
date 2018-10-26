#pragma once
#include "stormancer/headers.h"
#include "TeamsTypes.h"

namespace Stormancer
{
	class Scene;

	class TeamsService
	{
	public:

#pragma region public_methods

		TeamsService(std::shared_ptr<Scene> scene);
		
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

#pragma endregion

	private:

#pragma region
	
		std::weak_ptr<Scene> _scene;

#pragma endregion
	};
}
