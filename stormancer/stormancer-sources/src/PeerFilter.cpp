#include "stormancer/stdafx.h"
#include "stormancer/PeerFilter.h"
#include "stormancer/Serializer.h"
#include <algorithm>

namespace Stormancer
{
	PeerFilter PeerFilter::matchSceneHost()
	{
		return PeerFilter(PeerFilterType::MatchSceneHost);
	}

	PeerFilter PeerFilter::matchAllP2P()
	{
		return PeerFilter(PeerFilterType::MatchAllP2P);
	}

	PeerFilter PeerFilter::matchPeers(const std::vector<std::string>& ids)
	{
		return PeerFilter(ids);
	}

	PeerFilter PeerFilter::readFilter(ibytestream& stream)
	{
		Serializer serializer;
		PeerFilterType peerType = (PeerFilterType)serializer.deserializeOne<byte>(stream);
		switch (peerType)
		{
		case PeerFilterType::MatchPeers:
		{
			std::vector<std::string> peers = serializer.deserializeOne<std::vector<std::string>>(stream);
			return PeerFilter(peers);
		}
		case PeerFilterType::MatchAllP2P:
		{
			return PeerFilter(PeerFilterType::MatchAllP2P);
		}
		default:
		{
			return PeerFilter(PeerFilterType::MatchSceneHost);
		}
		}
	}

	PeerFilter::PeerFilter()
		: type(PeerFilterType::MatchSceneHost)
	{
	}

	PeerFilter::PeerFilter(PeerFilterType type)
		: type(type)
	{
	}

	PeerFilter::PeerFilter(const std::string& id)
		: type(PeerFilterType::MatchPeers)
		, ids(std::vector<std::string>(1, id))
	{
	}

	PeerFilter::PeerFilter(const std::vector<std::string>& ids)
		: type(PeerFilterType::MatchPeers)
		, ids(ids)
	{
	}

	bool PeerFilter::operator==(const PeerFilter& other) const
	{
		if (type != other.type)
		{
			return false;
		}

		for (size_t i = 0; i < ids.size(); i++)
		{
			if (std::find(other.ids.begin(), other.ids.end(), ids[i]) == other.ids.end())
			{
				return false;
			}
		}

		for (size_t i = 0; i < other.ids.size(); i++)
		{
			if (std::find(ids.begin(), ids.end(), other.ids[i]) == ids.end())
			{
				return false;
			}
		}

		return true;
	}

	bool PeerFilter::operator==(PeerFilterType otherType) const
	{
		return (type == otherType);
	}
}
