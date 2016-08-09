// dllmain.cpp : Defines the entry point for the DLL application.

#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#if defined(_WIN32)
#include "RakNetTypes.h"
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	RakNet::SystemAddress addr;
	addr.FromString("127.0.0.1|8080");
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

#endif // defined(_WIN32)
