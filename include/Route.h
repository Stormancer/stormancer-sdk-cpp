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
		uint16 index;
		vector< function<void(Packet<>*)>* > handlers;

	private:
		Scene* _scene;
		wstring _name;
		stringMap _metadata;
	};
};
