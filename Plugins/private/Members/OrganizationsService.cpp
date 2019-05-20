#include "OrganizationsService.h"

#include "OrganizationsService.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/Service.h"

namespace Stormancer
{
	OrganizationsService::OrganizationsService(std::shared_ptr<Scene> scene)
		: _scene(scene)
	{
	}

	pplx::task<Organization> OrganizationsService::getOrganization(const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<Organization>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<Organization>("organizations.getorganization", organizationId);
	}

	pplx::task<std::vector<Organization>> OrganizationsService::getOrganizations(const std::string& nameContains, int size, int skip)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<std::vector<Organization>>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<std::vector<Organization>>("organizations.getorganizations", nameContains, size, skip);
	}

	pplx::task<std::vector<Organization>> OrganizationsService::getUserOrganizations(const std::string& userId, bool showApplyingMembers)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<std::vector<Organization>>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<std::vector<Organization>>("organizations.getuserorganizations", userId, showApplyingMembers);
	}

	pplx::task<std::vector<Member>> OrganizationsService::getMembers(const std::string& organizationId, bool showApplyingMembers)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<std::vector<Member>>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<std::vector<Member>>("organizations.getmembers", organizationId, showApplyingMembers);
	}

	pplx::task<Organization> OrganizationsService::createOrganization(const std::string& organizationName, const std::string& customData)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<Organization>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<Organization>("organizations.createorganization", organizationName, customData);
	}

	pplx::task<void> OrganizationsService::updateOrganizationCustomData(const std::string& organizationId, const std::string& customData)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.updateorganizationcustomdata", organizationId, customData);
	}

	pplx::task<void> OrganizationsService::deleteOrganization(const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.deleteorganization", organizationId);
	}

	pplx::task<void> OrganizationsService::changeOrganizationOwner(const std::string& newOwnerId, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.changeowner", newOwnerId, organizationId);
	}

	pplx::task<void> OrganizationsService::inviteInOrganization(const std::string& userId, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.invite", userId, organizationId);
	}

	pplx::task<void> OrganizationsService::removeMember(const std::string& memberId, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.removemember", memberId, organizationId);
	}

	pplx::task<void> OrganizationsService::manageOrganizationInvitation(bool accept, const std::string& userId, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.manageinvitation", accept, userId, organizationId);
	}

	pplx::task<void> OrganizationsService::createOrganizationRole(const Role& role, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.createrole", role, organizationId);
	}

	pplx::task<void> OrganizationsService::deleteOrganizationRole(const std::string& roleName, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.deleterole", roleName, organizationId);
	}

	pplx::task<void> OrganizationsService::grantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.grantrole", roleName, userId, organizationId);
	}

	pplx::task<void> OrganizationsService::ungrantOrganizationRole(const std::string& roleName, const std::string& userId, const std::string& organizationId)
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Scene not available"));
		}

		auto rpcService = scene->dependencyResolver()->resolve<RpcService>();

		return rpcService->rpc<void>("organizations.ungrantrole", roleName, userId, organizationId);
	}
}
