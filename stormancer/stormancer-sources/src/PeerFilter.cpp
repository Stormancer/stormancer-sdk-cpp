#include "stormancer/stdafx.h"
#include "stormancer/PeerFilter.h"
#include "stormancer/Serializer.h"
#include <algorithm>

namespace Stormancer
{
	PeerFilter::PeerFilter()
		: type(PeerFilterType::MatchSceneHost)
	{
	}

	PeerFilter::PeerFilter(PeerFilterType type)
		: type(type)
	{
	}

	PeerFilter::PeerFilter(PeerFilterType type, const int64 id)
		: type(type)
		, ids(std::vector<int64>(1, id))
	{
	}

	PeerFilter::PeerFilter(PeerFilterType type, const std::vector<int64> ids)
		: type(type)
		, ids(ids)
	{
	}

	PeerFilter PeerFilter::readFilter(ibytestream& stream)
	{
		Serializer serializer;
		PeerFilterType peerType = (PeerFilterType)serializer.deserializeOne<byte>(stream);
		switch (peerType)
		{
		case PeerFilterType::MatchPeers:
		{
			std::vector<int64> peers = serializer.deserializeOne<std::vector<int64>>(stream);
			return PeerFilter(PeerFilterType::MatchPeers, peers);
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

	PeerFilter MatchSceneHost()
	{
		return PeerFilter(PeerFilterType::MatchSceneHost);
	}

	PeerFilter MatchPeers(const int64 id)
	{
		return PeerFilter(PeerFilterType::MatchPeers, id);
	}

	PeerFilter MatchPeers(const std::vector<int64> ids)
	{
		return PeerFilter(PeerFilterType::MatchPeers, ids);
	}

	PeerFilter MatchAllP2P()
	{
		return PeerFilter(PeerFilterType::MatchAllP2P);
	}
}
