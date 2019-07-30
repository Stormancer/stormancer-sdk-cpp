#include "stormancer/stdafx.h"
#include "stormancer/BuildConfig.h"
#include "stormancer/Version.h"

#define STORM_VERSION_CHECK_IMPL(version) checkVersionIs_ ## version
#define STORM_VERSION_CHECK(version) STORM_VERSION_CHECK_IMPL(version)

namespace Stormancer
{
	static const char* VersionString = STORM_VERSION;

	const char* Version::getVersionString() { return VersionString; }

	// Implement the method used for checking the version.
	// It is referenced in Configuration::create().
	// If there is a mismatch between the version in the header files and the one in the static library where this file resides,
	// there will be a linker error.
	void Version::STORM_VERSION_CHECK(STORM_VERSION_HASH)() {}
}