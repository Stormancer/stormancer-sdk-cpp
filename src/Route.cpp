#include "stormancer.h"

namespace Stormancer
{
	Route::Route()
		: _scene(nullptr),
		_handle(0)
	{
		cout << ">> Route ctor 1" << endl;
	}

	Route::Route(Scene* scene, wstring& routeName, uint16 handle, stringMap& metadata)
		: _name(routeName),
		_scene(scene),
		_metadata(metadata),
		_handle(handle)
	{
		cout << ">> Route ctor 2" << endl;
	}

	Route::Route(Scene* scene, wstring& routeName, stringMap& metadata)
		: Route(scene, routeName, 0, metadata)
	{
		cout << ">> Route ctor 3" << endl;
	}

	Route::~Route()
	{
		cout << ">> Route dtor" << endl;
	}

	Scene* Route::scene()
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
