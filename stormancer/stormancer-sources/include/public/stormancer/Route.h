#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/MessageOriginFilter.h"
#include "stormancer/StormancerTypes.h"
#include <unordered_map>
#include <list>
#include <string>
#include <memory>
#include <functional>

namespace Stormancer
{
	class IConnection;
	template<typename T>
	class Packet;

	

	/// Represents a route on a scene.
	class Route
	{
	public:

		Route();

		Route(const std::string& routeName, uint16 handle, MessageOriginFilter filter, std::unordered_map<std::string, std::string> metadata = std::unordered_map<std::string, std::string>());

		Route(const std::string& routeName, std::unordered_map<std::string, std::string> metadata = std::unordered_map<std::string, std::string>());

		virtual ~Route();

		const std::string& name() const;

		uint16 handle() const;

		const std::unordered_map<std::string, std::string>& metadata() const;

		void setHandle(uint16 newHandle);

		MessageOriginFilter filter() const;

		std::list<std::function<void(std::shared_ptr<Packet<IConnection>>)>> handlers;

	private:

		uint16 _handle;

		MessageOriginFilter _filter;

		std::string _name;

		std::unordered_map<std::string, std::string> _metadata;
	};

	using Route_ptr = std::shared_ptr<Route>;
}
