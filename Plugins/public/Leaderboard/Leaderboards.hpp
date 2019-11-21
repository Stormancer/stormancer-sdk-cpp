#pragma once
#include "Users/ClientAPI.hpp"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	namespace Leaderboards
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
			std::string path;
			float value = 0;

			MSGPACK_DEFINE(type, path, value);
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

			///<summary>
			/// Path in the scores object to use for ranking in the query.
			///</summary>
			std::string scorePath;


			MSGPACK_DEFINE(startId, scoreFilters, fieldFilters, size, skip, leaderboardName, friendsIds, order, scorePath);
		};
		template<typename T>
		struct Score
		{


			std::string id;
			T scores;
			int64 createdOn = 0;
			std::string document;

			MSGPACK_DEFINE(id, scores, createdOn, document);
		};
		template<typename T>
		struct LeaderboardRanking
		{
			int32 ranking = 0;
			Score<T> document;

			MSGPACK_DEFINE(ranking, document);
		};

		template<typename T>
		struct LeaderboardResult
		{
			std::string leaderboardName;
			std::vector<LeaderboardRanking<T>> results;
			std::string next;
			std::string previous;
			int64 total;

			MSGPACK_DEFINE(leaderboardName, results, next, previous, total);
		};



		class LeaderboardPlugin;

		namespace details
		{
			class LeaderboardService
			{
			public:



				LeaderboardService(std::weak_ptr<Scene> scene, std::shared_ptr<RpcService> rpc)
				{
					_scene = scene;

					_rpcService = rpc;

				}
				//Query a leaderboard
				template<typename T>
				pplx::task<LeaderboardResult<T>> query(LeaderboardQuery query)
				{
					return _rpcService->rpc<LeaderboardResult<T>>("leaderboard.query", query);
				}

				//Query a leaderboard using a cursor obtained from a LeaderboardResult (result.next or result.previous)
				template<typename T>
				pplx::task<LeaderboardResult<T>> query(const std::string& cursor)
				{
					return _rpcService->rpc<LeaderboardResult<T>>("leaderboard.cursor", cursor);
				}


			private:


				std::weak_ptr<Scene> _scene;
				std::shared_ptr<RpcService> _rpcService;

			};


		}

		class Leaderboard : public Stormancer::ClientAPI<Leaderboard, details::LeaderboardService>
		{
		public:

			Leaderboard(std::weak_ptr<Stormancer::Users::UsersApi> users)
				: Stormancer::ClientAPI<Leaderboard, details::LeaderboardService>(users, "stormancer.plugins.leaderboards")
			{
			}

			~Leaderboard() {}

			//Query a leaderboard
			template<typename T>
			pplx::task<LeaderboardResult<T>> query(LeaderboardQuery query)
			{
				return getLeaderboardService()
					.then([query](std::shared_ptr<Stormancer::Leaderboards::details::LeaderboardService> service)
						{
							return service->query<T>(query);
						});
			}

			template<typename T>
			//Query a leaderboard using a cursor obtained from a LeaderboardResult (result.next or result.previous)
			pplx::task<LeaderboardResult<T>> query(const std::string& cursor)
			{
				return getLeaderboardService()
					.then([cursor](std::shared_ptr<Stormancer::Leaderboards::details::LeaderboardService> service)
						{
							return service->query<T>(cursor);
						});
			}

		private:

			pplx::task<std::shared_ptr<Stormancer::Leaderboards::details::LeaderboardService>> getLeaderboardService()
			{
				return this->getService();
			}
		};

		class LeaderboardPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{

				if (scene)
				{
					auto name = scene->getHostMetadata("stormancer.leaderboard");

					if (!name.empty())
					{
						builder.registerDependency<Stormancer::Leaderboards::details::LeaderboardService, Scene, RpcService>().singleInstance();
					}
				}

			}
			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<Stormancer::Leaderboards::Leaderboard, Users::UsersApi>().as<Leaderboard>().singleInstance();
			}

		};

	}

}


MSGPACK_ADD_ENUM(Stormancer::Leaderboards::ComparisonOperator);
MSGPACK_ADD_ENUM(Stormancer::Leaderboards::LeaderboardOrdering);