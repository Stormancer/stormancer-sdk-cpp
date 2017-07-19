#pragma once

#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	class Scene;

	enum class MessageOriginFilter
	{
		Host = 1,
		Peer = 2,
		All = 0xFF
	};

	/// Represents a route on a scene.
	class Route
	{
	public:
		Route();
		Route(const std::string& routeName, uint16 handle, MessageOriginFilter filter, std::map<std::string, std::string> metadata = std::map<std::string, std::string>());
		Route(const std::string& routeName, std::map<std::string, std::string> metadata = std::map<std::string, std::string>());
		virtual ~Route();

	public:
		STORMANCER_DLL_API const std::string& name() const;
		STORMANCER_DLL_API uint16 handle() const;
		STORMANCER_DLL_API const std::map<std::string, std::string>& metadata() const;
		void setHandle(uint16 newHandle);
		MessageOriginFilter filter() const;

	public:
		std::list<std::function<void(Packet_ptr)>> handlers;

	private:
		uint16 _handle;
		std::weak_ptr<Scene> _scene;
		MessageOriginFilter _filter;
		std::string _name;
		std::map<std::string, std::string> _metadata;
	};

	using Route_ptr = std::shared_ptr<Route>;
};
