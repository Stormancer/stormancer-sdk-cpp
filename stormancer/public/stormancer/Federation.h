#pragma once
#include <string>
#include <vector>

namespace Stormancer
{
	struct Cluster
	{
		std::string id;
		std::vector<std::string> endpoints;
		std::vector<std::string> tags;
	};
	struct Federation
	{
		Cluster current;
		std::vector<Cluster> clusters;
		Cluster getCluster(std::string id);
	};
}