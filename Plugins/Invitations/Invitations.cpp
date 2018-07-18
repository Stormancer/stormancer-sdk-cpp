#include "stormancer/headers.h"
#include "stormancer/RPC/RpcService.h"
#include "Invitations.h"

Stormancer::InvitationsService::InvitationsService(Scene_ptr scene, std::shared_ptr<ILogger> logger) :
	_scene(scene),
	_logger(logger)
{
	this->onInvitationRequest = [](Invitation r) { 
		InvitationResponse response;
		response.accepted = true;
		return pplx::task_from_result(response);
	};

	this->onInvitationSucceded = [](InvitationData r) {};

	scene->addRoute("invitations.ready", [this](Stormancer::Packetisp_ptr packet) {
		auto result = _serializer.deserializeOne<InvitationData>(packet->stream);
		this->onInvitationSucceded(result);
	});

	scene->dependencyResolver().lock()->resolve<RpcService>()->addProcedure("invitations.onInviteRequest", [=](Stormancer::RpcRequestContext_ptr ctx) {
		auto result = _serializer.deserializeOne<Invitation>(ctx->inputStream());
		return this->onInvitationRequest(result).then([=](InvitationResponse response) {
			ctx->sendValue([=, &response](Stormancer::obytestream* stream) {
				_serializer.serialize(stream, response);
			});
		});
	});
}

pplx::task<Stormancer::InvitationData> Stormancer::InvitationsService::invite(std::string userId, std::string userData)
{
	auto scene = _scene.lock();
	if (!scene)
	{
		return pplx::task_from_exception<InvitationData>(std::runtime_error(""));
	}

	return scene->dependencyResolver().lock()->resolve<RpcService>()->rpc<Stormancer::InvitationData, std::string, std::string>("gameinvitation.invite", userId, userData);
}
