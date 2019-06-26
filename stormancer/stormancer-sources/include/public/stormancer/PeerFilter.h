#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Streams/bytestream.h"
#include "stormancer/StormancerTypes.h"
#include <vector>
#include <string>

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

#pragma region static_functions

		static PeerFilter matchSceneHost();

		static PeerFilter matchAllP2P();

		template<typename... TArgs>
		static PeerFilter matchPeers(const TArgs&... ids)
		{
			std::vector<std::string> vectorIds;
			return matchPeersInternal(vectorIds, ids...);
		}

		static PeerFilter matchPeers(const std::vector<std::string>& ids);

		static PeerFilter readFilter(ibytestream& stream);

#pragma endregion

#pragma region public_methods

		PeerFilter();

		PeerFilter(PeerFilterType type);

		PeerFilter(const std::string& id);

		PeerFilter(const std::vector<std::string>& ids);

		virtual ~PeerFilter() = default;

		bool operator==(const PeerFilter& other) const;

		bool operator==(PeerFilterType type) const;

#pragma endregion

#pragma region public_members

		const PeerFilterType type = PeerFilterType::MatchSceneHost;

		const std::vector<std::string> ids;

#pragma endregion

	private:

#pragma region private_static_functions

		template<typename... TArgs>
		static PeerFilter matchPeersInternal(std::vector<std::string>& vectorIds, const std::string& id, const TArgs&... ids)
		{
			vectorIds.emplace_back(id);
			return matchPeersInternal(vectorIds, ids...);
		}

		static PeerFilter matchPeersInternal(std::vector<std::string>& vectorIds)
		{
			return PeerFilter(vectorIds);
		}

#pragma endregion

	};
}
