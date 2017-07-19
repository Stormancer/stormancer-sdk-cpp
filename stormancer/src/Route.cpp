#include "stdafx.h"
#include "Route.h"

namespace Stormancer
{
	Route::Route()
		: _handle(0)
	{
	}

	Route::Route(const std::string& routeName, uint16 handle, MessageOriginFilter filter, std::map<std::string, std::string> metadata)
		: _name(routeName),
		_metadata(metadata),
		_handle(handle),
		_filter(filter)
	{
	}

	Route::Route(const std::string& routeName, std::map<std::string, std::string> metadata)
		: Route(routeName, 0, MessageOriginFilter::Host, metadata)
	{
	}

	Route::~Route()
	{
	}

	const std::string& Route::name() const
	{
		return _name;
	}

	uint16 Route::handle() const
	{
		return _handle;
	}

	const std::map<std::string, std::string>& Route::metadata() const
	{
		return _metadata;
	}

	void Route::setHandle(uint16 newHandle)
	{
		_handle = newHandle;
	}

	MessageOriginFilter Route::filter() const
	{
		return _filter;
	}
};
