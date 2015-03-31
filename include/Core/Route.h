#pragma once
#include "headers.h"

namespace Stormancer
{
	class Route
	{
	public:
		Route(IScene* scene, wstring routeName, uint16 handle, stringMap metadata = stringMap());
		Route(IScene* scene, wstring routeName, stringMap metadata = stringMap());
		virtual ~Route();

	public:
		IScene* scene();
		wstring name();
		stringMap metadata();

	public:
		uint16 index;
		function<void(Packet<>)> handlers;

	private:
		IScene* _scene;
		wstring _name;
		stringMap _metadata;
	};
};
