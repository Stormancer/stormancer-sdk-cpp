#pragma once
#include "headers.h"

namespace Stormancer
{
	class Scene;

	class Route
	{
	public:
		Route();
		Route(Scene* scene, wstring& routeName, uint16 handle, stringMap& metadata = stringMap());
		Route(Scene* scene, wstring& routeName, stringMap& metadata = stringMap());
		virtual ~Route();

	public:
		Scene* scene();
		wstring name();
		stringMap metadata();

	public:
		wstring _name;
		uint16 _handle;
		Scene* _scene;
		stringMap _metadata;
		vector< function<void(Packet<>*)>* > handlers;
	};
};
