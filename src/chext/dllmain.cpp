#include "stdafx.h"
#include "chext.h"
#include "dllmain.h"
#include "MenuExt.h"
#include "DropMenuExt.h"
#include "ShellExtControl.h"

CCHExtModule _AtlModule;

OBJECT_ENTRY_AUTO(CLSID_MenuExt, CMenuExt)
OBJECT_ENTRY_AUTO(CLSID_DropMenuExt, CDropMenuExt)
OBJECT_ENTRY_AUTO(CLSID_CShellExtControl, CShellExtControl)

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hInstance);

	return _AtlModule.DllMain(dwReason, lpReserved);
}
