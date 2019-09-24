#pragma once

#include <string>
#include <vector>
#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"

namespace Stormancer
{

	enum class ComparisonOperator : int8
	{
		GREATER_THAN_OR_EQUAL = 0,
		GREATER_THAN = 1,
		LESSER_THAN_OR_EQUAL = 2,
		LESSER_THAN = 3
	};

	enum class LeaderboardOrdering : int8
	{
		Ascending = 0,
		Descending = 1
	};
	
	struct ScoreFilter
	{
		ComparisonOperator type;
		int32 value = 0;

		MSGPACK_DEFINE(type, value);
	};

	struct FieldFilter
	{
		std::string field;
		std::vector<std::string> values;

		MSGPACK_DEFINE(field, values);
	};

	struct LeaderboardQuery
	{
		std::string startId;
		std::vector<ScoreFilter> scoreFilters;
		std::vector<FieldFilter> fieldFilters;
		int32 size = 1;
		int32 skip = 0;
		std::string leaderboardName;
		std::vector<std::string> friendsIds;
		LeaderboardOrdering order = LeaderboardOrdering::Descending;

		MSGPACK_DEFINE(startId, scoreFilters, fieldFilters, size, skip, leaderboardName, friendsIds, order);
	};

	struct Score
	{
		std::string getAttachmentUrl();

		std::string id;
		int32 score = 0;
		int64 createdOn = 0;
		std::string document;
		
		MSGPACK_DEFINE(id, score, createdOn, document);
	};

	struct LeaderboardRanking
	{
		int32 ranking = 0;
		Score document;

		MSGPACK_DEFINE(ranking, document);
	};

	struct LeaderboardResult
	{
		std::string leaderboardName;
		std::vector<LeaderboardRanking> results;
		std::string next;
		std::string previous;
		int64 total;

		MSGPACK_DEFINE(leaderboardName, results, next, previous, total);
	};
}

MSGPACK_ADD_ENUM(Stormancer::ComparisonOperator);
MSGPACK_ADD_ENUM(Stormancer::LeaderboardOrdering);
