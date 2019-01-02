#pragma once
#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"
#include <string>

namespace Stormancer
{
	enum class InAppNotificationAcknowledgment : int8
	{
		None = 0,
		OnSend = 1,
		OnReceive = 2,
		ByUser = 3
	};

	struct InAppNotification
	{
		std::string id;
		std::string type;
		std::string userId;
		std::string message;
		std::string data;
		int64 createdOn;
		bool shouldExpire;
		int64 expirationDate;
		InAppNotificationAcknowledgment Acknowledgment;

		MSGPACK_DEFINE(id, type, userId, message, data, createdOn, shouldExpire, expirationDate, Acknowledgment);
	};
}

MSGPACK_ADD_ENUM(Stormancer::InAppNotificationAcknowledgment);
