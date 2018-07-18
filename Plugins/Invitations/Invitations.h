#pragma once

#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	struct InvitationData
	{
		std::string userData;
		std::string connectionToken;
		MSGPACK_DEFINE(userData, connectionToken);
	};

	struct Invitation
	{
		std::string senderId;
		std::string userData;

		MSGPACK_DEFINE(senderId, userData);
	};

	struct InvitationResponse
	{
		std::string userData;
		bool accepted;

		MSGPACK_DEFINE(userData, accepted);
	};

	class InvitationsService
	{
	public:

		InvitationsService(Scene_ptr scene, std::shared_ptr<ILogger> logger);
		pplx::task<InvitationData> invite(std::string userId, std::string userData);

		std::function<void(InvitationData)> onInvitationSucceded;

		std::function<pplx::task<InvitationResponse>(Invitation)> onInvitationRequest;

	private:
		
		std::weak_ptr<Scene> _scene;
		ILogger_ptr _logger;
		Serializer _serializer;
	};
}
