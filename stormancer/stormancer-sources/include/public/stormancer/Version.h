#pragma once

#include "stormancer/BuildConfig.h"
#include <cstring>

#ifndef STORM_VERSION
#define STORM_VERSION ""
#endif
#ifndef STORM_VERSION_HASH
#define STORM_VERSION_HASH noversion
#endif

#define STORM_VERSION_CHECK_IMPL(version) checkVersionIs_ ## version
#define STORM_VERSION_CHECK(version) STORM_VERSION_CHECK_IMPL(version)

#if defined (STORM_CHECK_VERSION_LINKTIME) && !defined(STORM_DISABLE_CHECK_VERSION_LINKTIME)
#define STORM_DO_CHECK_VERSION
#endif

namespace Stormancer
{
	class Version
	{
	public:

		/// <summary>
		/// Get the version of this Stormancer client library.
		/// </summary>
		/// <remarks>
		/// The version returned by this function has the standard git-describe format : (latest version tag)-(number of commits since this version)-g(short hash of the current commit)
		/// </remarks>
		/// <returns>The version specification, as a C-string. This is a pointer to static data.</returns>
		static const char* getVersionString();

	private:

		friend class Configuration;
		/// <summary>
	#if defined STORM_DO_CHECK_VERSION || defined _STORMANCERSDKCPP
		// Deckaration of the method defined in Version.cpp
		static void STORM_VERSION_CHECK(STORM_VERSION_HASH)();
	#endif

		/// <summary>
		/// Verify at link time that the version of the header files and the version of the library are the same.
		/// </summary>
		/// <remarks>
		/// This check is in effect only if you also define the STORM_CHECK_VERSION_LINKTIME macro in your project. Otherwise, it does nothing.
		/// When the macro is defined, this check will cause a linker error if the headers version and the library version are different.
		/// This is implemented with an empty function that contains the version hash in its name, which is defined in Version.cpp.
		/// If the version of the headers is different from the library's, the symbol will be undefined, causing a link failure.
		/// To find out the version of the library, dump its symbols and look for <c>Stormancer::VersionString</c>.
		/// </remarks>
		static void checkVersionLinkTime()
		{
	#ifdef STORM_DO_CHECK_VERSION
			STORM_VERSION_CHECK(STORM_VERSION_HASH)();
	#endif
		}

		static const char* getHeadersVersionString()
		{
			return STORM_VERSION;
		}
	};
}

#undef STORM_VERSION_CHECK_IMPL
#undef STORM_VERSION_CHECK
#undef STORM_DO_CHECK_VERSION
#ifndef _STORMANCERSDKCPP
// These macros should stay private to the library.
// User code should only see Version::getVersionString().
#undef STORM_VERSION
#undef STORM_VERSION_HASH
#endif // !STORMANCERSDKCPP
