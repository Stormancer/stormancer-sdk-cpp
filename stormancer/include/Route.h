#pragma once
#include "headers.h"

namespace Stormancer
{
	class Scene;

	/// Represents a route on a scene.
	class Route
	{
	public:
		Route();
		Route(Scene* scene, std::string& routeName, uint16 handle, stringMap metadata = stringMap());
		Route(Scene* scene, std::string& routeName, stringMap metadata = stringMap());
		virtual ~Route();

	public:
		Scene* scene();
		STORMANCER_DLL_API const std::string name();
		STORMANCER_DLL_API uint16 handle();
		STORMANCER_DLL_API stringMap& metadata();
		void setHandle(uint16 newHandle);

	public:
		std::list<std::function<void(Packet_ptr)>> handlers;

	private:
		uint16 _handle;
		Scene* _scene;
		std::string _name;
		stringMap _metadata;
	};

	using Route_ptr = std::shared_ptr<Route>;
};
