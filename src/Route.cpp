#include "stormancer.h"

namespace Stormancer
{
	Route::Route()
		: _handle(0)
	{
	}

	Route::Route(Scene* scene, std::string& routeName, uint16 handle, stringMap metadata)
		: _name(routeName),
		_scene(scene),
		_metadata(metadata),
		_handle(handle)
	{
	}

	Route::Route(Scene* scene, std::string& routeName, stringMap metadata)
		: Route(scene, routeName, 0, metadata)
	{
	}

	Route::~Route()
	{
	}

	Scene* Route::scene()
	{
		return _scene;
	}

	const char* Route::name()
	{
		return _name.c_str();
	}

	uint16 Route::handle()
	{
		return _handle;
	}

	stringMap& Route::metadata()
	{
		return _metadata;
	}

	void Route::setHandle(uint16 newHandle)
	{
		_handle = newHandle;
	}
};
