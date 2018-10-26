#include "stormancer/headers.h"
#include "Teams.h"
#include "TeamsContainer.h"
#include "TeamsService.h"
#include "stormancer/Scene.h"
#include "stormancer/SafeCapture.h"
#include "../Authentication/AuthenticationService.h"

namespace Stormancer
{
	pplx::task<std::shared_ptr<TeamsContainer>> Teams::initialize(std::string teamSceneName)
	{
		auto auth = _auth.lock();

		if (!auth)
		{
			return pplx::task_from_exception<std::shared_ptr<TeamsContainer>>(std::runtime_error("Authentication service destroyed."));
		}

		std::weak_ptr<Teams> wThat = this->shared_from_this();
		_teamsContainerTask = auth->getSceneForService("stormancer.plugins.matchmaking", teamSceneName).then([wThat](Scene_ptr scene) {
			auto that = wThat.lock();
			if (!that)
			{
				throw PointerDeletedException("Authentication service destroyed.");
			}

			auto container = std::make_shared<TeamsContainer>();
			container->_scene = scene;
			container->_connectionStateChangedSubscription = scene->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState s) {
				if (s == ConnectionState::Disconnected)
				{
					if (auto that = wThat.lock())
					{
						that->_teamsContainerTask = pplx::task_from_result<std::shared_ptr<TeamsContainer>>(nullptr);
					}
				}
			});
			return container;

		});
		return _teamsContainerTask;
	}

	pplx::task<Team> Teams::getTeam(const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->getTeam(teamId);
		});
	}

	pplx::task<Team> Teams::getTeamByName(const std::string& teamName)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, teamName](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->getTeamByName(teamName);
		});
	}

	pplx::task<std::vector<TeamMember>> Teams::getMembers(const std::string& teamId, bool showApplyingMembers)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, teamId, showApplyingMembers](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->getMembers(teamId, showApplyingMembers);
		});
	}

	pplx::task<Team> Teams::createTeam(const std::string& teamName, const std::string& customData)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, teamName, customData](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->createTeam(teamName, customData);
		});
	}

	pplx::task<void> Teams::updateTeamCustomData(const std::string& teamId, const std::string& customData)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, teamId, customData](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->updateTeamCustomData(teamId, customData);
		});
	}

	pplx::task<void> Teams::deleteTeam(const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->deleteTeam(teamId);
		});
	}

	pplx::task<void> Teams::changeTeamOwner(const std::string& newOwnerId, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, newOwnerId, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->changeTeamOwner(newOwnerId, teamId);
		});
	}

	pplx::task<void> Teams::inviteInTeam(const std::string& userId, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, userId, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->inviteInTeam(userId, teamId);
		});
	}

	pplx::task<void> Teams::removeMember(const std::string& memberId, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, memberId, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->removeMember(memberId, teamId);
		});
	}

	pplx::task<void> Teams::manageTeamInvitation(bool accept, const std::string& userId, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, accept, userId, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->manageTeamInvitation(accept, userId, teamId);
		});
	}

	pplx::task<void> Teams::createTeamRole(const Role& role, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, role, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->createTeamRole(role, teamId);
		});
	}

	pplx::task<void> Teams::deleteTeamRole(const std::string& roleName, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, roleName, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->deleteTeamRole(roleName, teamId);
		});
	}

	pplx::task<void> Teams::grantTeamRole(const std::string& roleName, const std::string& userId, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, roleName, userId, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->grantTeamRole(roleName, userId, teamId);
		});
	}

	pplx::task<void> Teams::ungrantTeamRole(const std::string& roleName, const std::string& userId, const std::string& teamId)
	{
		std::weak_ptr<Teams> wTeams = this->shared_from_this();
		return _teamsContainerTask.then([wTeams, roleName, userId, teamId](std::shared_ptr<TeamsContainer> teamsContainer) {
			auto teams = wTeams.lock();
			if (!teams)
			{
				throw PointerDeletedException("Teams deleted");
			}

			return teamsContainer->service()->grantTeamRole(roleName, userId, teamId);
		});
	}
}
