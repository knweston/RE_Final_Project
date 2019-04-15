// dllmain.cpp : Defines the entry point for the DLL application.

#include "ReflectiveLoader.h"
#include "Solver.h"

extern HINSTANCE hAppInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved)
{
	GameStats stats;
	BOOL bReturnValue = TRUE;
	switch (dwReason)
	{
	case DLL_QUERY_HMODULE:
		if (lpReserved != NULL)
			*(HMODULE *)lpReserved = hAppInstance;
		break;
	case DLL_PROCESS_ATTACH:
		hAppInstance = hinstDLL;
		MessageBoxA(NULL, "Code Injected!", "Message Box", MB_OK);
		stats.getInfo();
		stats.print();
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return bReturnValue;
}