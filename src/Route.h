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
		std::string name();
		stringMap metadata();

	public:
		std::string _name;
		uint16 _handle;
		Scene* _scene;
		stringMap _metadata;
		std::vector< std::function<void(std::shared_ptr<Packet<>>)>* > handlers;
	};
};
