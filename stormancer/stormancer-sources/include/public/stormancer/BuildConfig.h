#pragma once

/*
	This file should contain only the preprocessor definitions needed by stormancer.
	It also contains linker directives for the system libraries required by stormancer.
	It should be included at the top of every header file that is part of the stormancer lib.

	This file should not be directly included by client code.
*/

#if !defined(STORMANCER_DYNAMIC)

// Building a static lib (the default)

#ifndef _NO_ASYNCRTIMP
#define _NO_ASYNCRTIMP
#endif

#ifndef _NO_PPLXIMP
#define _NO_PPLXIMP
#endif

#ifndef _RAKNET_LIB
#define _RAKNET_LIB
#endif

#define STORMANCER_DLL_API

#else // !defined(STORMANCER_DYNAMIC)

// Building a dynamic lib

#ifdef STORMANCER_DLL_EXPORT
#define STORMANCER_DLL_API __declspec(dllexport)
#else
#define STORMANCER_DLL_API __declspec(dllimport)
#endif

#endif // !defined(STORMANCER_DYNAMIC)

// If pplx was included before this point, it must be with the same settings than the ones required by stormancer.
// Make sure this is the case
#if defined(CPPREST_FORCE_PPLX)


#if CPPREST_FORCE_PPLX != 1
#error "CPPREST_FORCE_PPLX must be set to 1 on this platform. Make sure you included stormancer/Tasks.h instead of pplx/pplxtask.h"
#endif
#endif





// System libraries required by Stormancer






























#pragma comment(lib, "ws2_32.lib")

#pragma comment(lib, "iphlpapi.lib") // Needed by libupnp
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Crypt32.lib")

#pragma comment(lib, "Bcrypt.lib")

