#include "stdafx.h"
#include "Helpers.h"
#include "ChannelUidStore.h"

namespace Stormancer
{
	ChannelUidStore::ChannelUidStore()
	{
		auto now = std::chrono::steady_clock::now();;
		for (int channelUid = 0; channelUid < channelUidsCount; channelUid++)
		{
			_channelsInfos[channelUid].lastUsed = now;
		}
	}

	int ChannelUidStore::getChannelUid(const std::string& channelIdentifier)
	{
		// Look for reserved channelUid by channelIdentifier

		int channelUid = getReservedChannel(channelIdentifier);
		if (channelUid == -1)
		{
			// Get the older used channel

			channelUid = getOlderChannel();

			// This has to return a correct value

			if (channelUid == -1)
			{
				throw std::runtime_error("No channelUid available");
			}

			// Reserve the channelUid for this channelIdentifier

			{
				std::lock_guard<std::mutex> lg(_channelsInfosMutex);

				_channelsInfos[channelUid].channelIdentifier = channelIdentifier;
			}
		}

		// Update the channelUid last used date

		{
			std::lock_guard<std::mutex> lg(_channelsInfosMutex);

			_channelsInfos[channelUid].lastUsed = std::chrono::steady_clock::now();
		}

		return channelUid;
	}

	int ChannelUidStore::getReservedChannel(const std::string& channelIdentifier)
	{
		std::lock_guard<std::mutex> lg(_channelsInfosMutex);

		for (int channelUid = 0; channelUid < channelUidsCount; channelUid++)
		{
			auto& channelInfo = _channelsInfos[channelUid];
			if (channelInfo.channelIdentifier == channelIdentifier)
			{
				return channelUid;
			}
		}

		return -1;
	}

	int ChannelUidStore::getOlderChannel()
	{
		int minChannelUid = -1;
		auto minDate = std::chrono::steady_clock::now();

		std::lock_guard<std::mutex> lg(_channelsInfosMutex);

		for (int channelUid = 0; channelUid < channelUidsCount; channelUid++)
		{
			auto& channelInfo = _channelsInfos[channelUid];
			if (channelInfo.lastUsed < minDate)
			{
				minDate = channelInfo.lastUsed;
				minChannelUid = channelUid;
			}
		}

		return minChannelUid;
	}
}
