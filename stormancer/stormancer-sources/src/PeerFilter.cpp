#include "stormancer/stdafx.h"
#include "stormancer/PeerFilter.h"

namespace Stormancer
{
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

	PeerFilter::~PeerFilter()
	{
	}
	
	PeerFilter PeerFilter::readFilter(ibytestream& stream)
	{
		byte peerType;
		stream >> peerType;
		switch (peerType)
		{
			case 0:
			{
				int64 id;
				stream >> id;
				return PeerFilter(PeerFilterType::MatchPeerFilter, id);
			}
			case 1:
			{
				uint16 nbPeers;
				stream >> nbPeers;
				std::vector<int64> peers(nbPeers);
				for (int i = 0; i < nbPeers; i++)
				{
					stream >> peers[i];
				}
				return PeerFilter(PeerFilterType::MatchArrayFilter, peers);
			}
			case 2:
			{
				return PeerFilter(PeerFilterType::MatchAllFilter);
			}
			default:
			{
				return PeerFilter(PeerFilterType::MatchSceneHost);
			}
		}
	}

	PeerFilter MatchAllFilter()
	{
		return PeerFilter(PeerFilterType::MatchAllFilter);
	}

	PeerFilter MatchSceneHost()
	{
		return PeerFilter(PeerFilterType::MatchSceneHost);
	}

	PeerFilter MatchPeerFilter(const int64 id)
	{
		return PeerFilter(PeerFilterType::MatchPeerFilter, id);
	}

	PeerFilter MatchArrayFilter(const std::vector<int64> ids)
	{
		return PeerFilter(PeerFilterType::MatchArrayFilter, ids);
	}
};
