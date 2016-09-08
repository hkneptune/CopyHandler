#include "stdafx.h"
#include "chext.h"
#include "dllmain.h"
#include "MenuExt.h"
#include "DropMenuExt.h"
#include "ShellExtControl.h"
#include "TLogger.h"

CCHExtModule _AtlModule;

OBJECT_ENTRY_AUTO(CLSID_MenuExt, CMenuExt)
OBJECT_ENTRY_AUTO(CLSID_DropMenuExt, CDropMenuExt)
OBJECT_ENTRY_AUTO(CLSID_CShellExtControl, CShellExtControl)

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);

		try
		{
			TLogger lg;
			BOOST_LOG_SEV(lg, debug) << L"DllMain - attaching to process: " << hInstance << L", " << dwReason << L", " << lpReserved;
		}
		catch (const std::exception&)
		{
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		try
		{
			TLogger lg;
			BOOST_LOG_SEV(lg, debug) << L"DllMain - detaching from process: " << hInstance << L", " << dwReason << L", " << lpReserved;
		}
		catch (const std::exception&)
		{

		}
	}

	return _AtlModule.DllMain(dwReason, lpReserved);
}
