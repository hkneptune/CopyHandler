#include "stdafx.h"
#include "MenuExt.h"
#include "DropMenuExt.h"
#include "ShellExtControl.h"

HINSTANCE g_hInstance = nullptr;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);
		g_hInstance = hInstance;
	}

	return TRUE;
}
