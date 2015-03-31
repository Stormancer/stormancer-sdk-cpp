#include "stormancer.h"

namespace Stormancer
{
	Route::Route(IScene* scene, wstring routeName, uint16 handle, stringMap metadata)
		: _name(routeName),
		_scene(scene),
		_metadata(metadata),
		index(handle)
	{
	}

	Route::Route(IScene* scene, wstring routeName, stringMap metadata)
		: Route(scene, routeName, 0, metadata)
	{
	}

	Route::~Route()
	{
	}

	IScene* Route::scene()
	{
		return _scene;
	}

	wstring Route::name()
	{
		return _name;
	}

	stringMap Route::metadata()
	{
		return _metadata;
	}
};
