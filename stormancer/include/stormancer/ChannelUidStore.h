#pragma once

#include "stormancer/headers.h"

namespace Stormancer
{
	class ChannelUidStore
	{
	public:

#pragma region public_methods

		ChannelUidStore();

		int getChannelUid(const std::string& channelIdentifier);

#pragma endregion

	private:

#pragma region private_classes

		struct ChannelInfos
		{
			std::string channelIdentifier;
			std::chrono::steady_clock::time_point lastUsed;
		};

#pragma endregion

#pragma region private_methods

		int getReservedChannel(const std::string& channelIdentifier);

		int getOlderChannel();

#pragma endregion

#pragma region private_members

		static const int channelUidsCount = 16;
		ChannelInfos _channelsInfos[channelUidsCount];
		std::mutex _channelsInfosMutex;

#pragma endregion
	};
}
