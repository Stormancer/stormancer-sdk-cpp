#pragma once
#include "headers.h"

namespace Stormancer
{
	class Scene;
	using Scene_ptr = std::shared_ptr<Scene>;
	using Scene_wptr = std::weak_ptr<Scene>;

	/// Represents a route on a scene.
	class Route
	{
	public:
		Route();
		Route(Scene_wptr scene, std::string& routeName, uint16 handle, stringMap metadata = stringMap());
		Route(Scene_wptr scene, std::string& routeName, stringMap metadata = stringMap());
		virtual ~Route();

	public:
		Scene_wptr scene();
		STORMANCER_DLL_API const char* name();
		STORMANCER_DLL_API uint16 handle();
		STORMANCER_DLL_API stringMap& metadata();
		void setHandle(uint16 newHandle);

	public:
		std::list<std::function<void(Packet_ptr)>> handlers;

	private:
		uint16 _handle;
		Scene_wptr _scene;
		std::string _name;
		stringMap _metadata;
	};

	using Route_ptr = std::shared_ptr<Route>;
};
