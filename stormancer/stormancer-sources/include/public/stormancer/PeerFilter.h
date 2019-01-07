#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Streams/bytestream.h"
#include "stormancer/stormancerTypes.h"
#include <vector>

namespace Stormancer
{
	enum class PeerFilterType
	{
		MatchAllFilter,
		MatchSceneHost,
		MatchPeerFilter,
		MatchArrayFilter
	};

	class PeerFilter
	{
	public:

		PeerFilter(PeerFilterType type);
		PeerFilter(PeerFilterType type, const int64 id);
		PeerFilter(PeerFilterType type, const std::vector<int64> ids);
		virtual ~PeerFilter();
		static PeerFilter readFilter(ibytestream* stream);

		const PeerFilterType type = PeerFilterType::MatchSceneHost;
		const std::vector<int64> ids;
	};

	PeerFilter MatchAllFilter();

	PeerFilter MatchSceneHost();

	PeerFilter MatchPeerFilter(const int64 id);

	PeerFilter MatchArrayFilter(const std::vector<int64> ids);
}
