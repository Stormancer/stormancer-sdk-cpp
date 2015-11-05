#include "stormancer.h"

namespace Stormancer
{
	Route::Route()
		: _handle(0)
	{
	}

	Route::Route(Scene_wptr scene, std::string& routeName, uint16 handle, stringMap metadata)
		: _name(routeName),
		_scene(scene),
		_metadata(metadata),
		_handle(handle)
	{
	}

	Route::Route(Scene_wptr scene, std::string& routeName, stringMap metadata)
		: Route(scene, routeName, 0, metadata)
	{
	}

	Route::~Route()
	{
	}

	Scene_wptr Route::scene()
	{
		return _scene;
	}

	std::string Route::name()
	{
		return _name;
	}

	stringMap& Route::metadata()
	{
		return _metadata;
	}
};
