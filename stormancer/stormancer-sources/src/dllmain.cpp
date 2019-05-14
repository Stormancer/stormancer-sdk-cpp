#include "stormancer/stdafx.h"
#include "stormancer/BuildConfig.h"

// dllmain.cpp : Defines the entry point for the DLL application.

#if defined(_WIN32) && defined(STORMANCER_DYNAMIC)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
#endif
// Windows Header Files:
#include <windows.h>


BOOL APIENTRY DllMain(HMODULE, DWORD  ul_reason_for_call, LPVOID)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif // defined(_WIN32) && !defined(_XBOX_ONE) && defined(STORMANCER_DYNAMIC)
