#include "stdafx.h"
#include "Invitations.h"

Stormancer::InvitationsService::InvitationsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger) :
	_scene(scene),
	_logger(logger)
{
	this->onInvitationRequest = [](Invitation r) { 
		InvitationResponse response;
		response.accepted = true;
		return pplx::task_from_result(response);
	};

	this->onInvitationSucceded = [](InvitationData r) {};

	_scene->addRoute("invitations.ready", [this](Stormancer::Packetisp_ptr packet) {
		auto result = _serializer.deserializeOne<InvitationData>(packet->stream);
		this->onInvitationSucceded(result);
	});

	_scene->dependencyResolver()->resolve<RpcService>()->addProcedure("invitations.onInviteRequest", [=](Stormancer::RpcRequestContext_ptr ctx) {
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
	return _scene->dependencyResolver()->resolve<RpcService>()->rpc<Stormancer::InvitationData, std::string, std::string>("gameinvitation.invite", userId, userData);
}