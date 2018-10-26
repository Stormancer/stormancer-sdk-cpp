#include "stormancer/headers.h"
#include "TeamsService.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/RpcService.h"

namespace Stormancer
{
	TeamsService::TeamsService(std::shared_ptr<Scene> scene)
		: _scene(scene)
	{
	}

	pplx::task<Team> TeamsService::getTeam(const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<Team>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<Team>("teams.getteam", teamId);
	}

	pplx::task<Team> TeamsService::getTeamByName(const std::string& teamName)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<Team>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<Team>("teams.getteambyname", teamName);
	}

	pplx::task<std::vector<TeamMember>> TeamsService::getMembers(const std::string& teamId, bool showApplyingMembers)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<std::vector<TeamMember>>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<std::vector<TeamMember>>("teams.getmembers", teamId, showApplyingMembers);
	}

	pplx::task<Team> TeamsService::createTeam(const std::string& teamName, const std::string& customData)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<Team>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<Team>("teams.createteam", teamName, customData);
	}

	pplx::task<void> TeamsService::updateTeamCustomData(const std::string& teamId, const std::string& customData)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.updateteamcustomdata", teamId, customData);
	}

	pplx::task<void> TeamsService::deleteTeam(const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.deleteteam", teamId);
	}

	pplx::task<void> TeamsService::changeTeamOwner(const std::string& newOwnerId, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.changeowner", newOwnerId, teamId);
	}

	pplx::task<void> TeamsService::inviteInTeam(const std::string& userId, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.invite", userId, teamId);
	}

	pplx::task<void> TeamsService::removeMember(const std::string& memberId, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.removemember", memberId, teamId);
	}

	pplx::task<void> TeamsService::manageTeamInvitation(bool accept, const std::string& userId, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.manageinvitation", accept, userId, teamId);
	}

	pplx::task<void> TeamsService::createTeamRole(const Role& role, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.createrole", role, teamId);
	}

	pplx::task<void> TeamsService::deleteTeamRole(const std::string& roleName, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.deleterole", roleName, teamId);
	}

	pplx::task<void> TeamsService::grantTeamRole(const std::string& roleName, const std::string& userId, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.grantrole", roleName, userId, teamId);
	}

	pplx::task<void> TeamsService::ungrantTeamRole(const std::string& roleName, const std::string& userId, const std::string& teamId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

		return rpcService->rpc<void>("teams.ungrantrole", roleName, userId, teamId);
	}
}
