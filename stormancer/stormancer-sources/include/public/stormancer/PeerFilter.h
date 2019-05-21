#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Streams/bytestream.h"
#include "stormancer/StormancerTypes.h"
#include <vector>

namespace Stormancer
{
	enum class PeerFilterType
	{
		MatchSceneHost = 0,
		MatchPeers = 1,
		MatchAllP2P = 2
	};

	class PeerFilter
	{
	public:

		static PeerFilter readFilter(ibytestream& stream);

		PeerFilter();
		PeerFilter(PeerFilterType type);
		PeerFilter(PeerFilterType type, const int64 id);
		PeerFilter(PeerFilterType type, const std::vector<int64> ids);
		virtual ~PeerFilter() = default;

		bool operator==(const PeerFilter& other) const;
		bool operator==(PeerFilterType type) const;

		const PeerFilterType type = PeerFilterType::MatchSceneHost;
		const std::vector<int64> ids;
	};
	
	PeerFilter MatchSceneHost();

	PeerFilter MatchPeers(const int64 id);

	PeerFilter MatchPeers(const std::vector<int64> ids);

	PeerFilter MatchAllP2P();
}
